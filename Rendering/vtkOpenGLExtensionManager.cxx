// -*- c++ -*-

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenGLExtensionManager.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

  Copyright 2003 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

=========================================================================*/

#include "vtkOpenGLExtensionManager.h"
#include "vtkOpenGLExtensionManagerConfigure.h"
#include "vtkgl.h"

#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"

#include <string.h>

#include <vtkstd/string>

#ifdef VTK_USE_GLX_GET_PROC_ADDRESS_ARB
typedef void (*__GLXextFuncPtr)(void);
extern "C" __GLXextFuncPtr glXGetProcAddressARB (const GLubyte *);
#endif //VTK_USE_GLX_GET_PROC_ADDRESS_ARB

#ifdef VTK_USE_VTK_DYNAMIC_LOADER
#include "vtkDynamicLoader.h"
#include <vtkstd/string>
#include <vtkstd/list>
#endif

#ifdef VTK_USE_APPLE_LOADER
#include <mach-o/dyld.h>
#endif //VTK_USE_APPLE_LOADER

// GLU is currently not linked in VTK.  We do not support it here.
#define GLU_SUPPORTED   0

vtkCxxRevisionMacro(vtkOpenGLExtensionManager, "1.2");
vtkStandardNewMacro(vtkOpenGLExtensionManager);

vtkOpenGLExtensionManager::vtkOpenGLExtensionManager()
{
  this->RenderWindow = NULL;
  this->ExtensionsString = NULL;

  this->Modified();
}

vtkOpenGLExtensionManager::~vtkOpenGLExtensionManager()
{
  this->SetRenderWindow(NULL);
  if (this->ExtensionsString)
    {
    delete[] this->ExtensionsString;
    }
}

void vtkOpenGLExtensionManager::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "RenderWindow: (" << this->RenderWindow << ")" << endl;
  os << indent << "BuildTime: " << this->BuildTime << endl;
  os << indent << "ExtensionsString: "
     << (this->ExtensionsString ? this->ExtensionsString : "(NULL)") << endl;
}

void vtkOpenGLExtensionManager::SetRenderWindow(vtkRenderWindow *renwin)
{
  if (renwin == this->RenderWindow)
    {
    return;
    }

  vtkDebugMacro("Setting RenderWindow to " << renwin);

  if (this->RenderWindow)
    {
    this->RenderWindow->UnRegister(this);
    }

  this->RenderWindow = renwin;
  if (this->RenderWindow)
    {
    this->RenderWindow->Register(this);
    }

  this->Modified();
}

void vtkOpenGLExtensionManager::Update()
{
  if (this->BuildTime > this->MTime)
    {
    return;
    }

  vtkDebugMacro("Update");

  if (this->ExtensionsString)
    {
    delete[] this->ExtensionsString;
    this->ExtensionsString = NULL;
    }

  this->ReadOpenGLExtensions();

  this->BuildTime.Modified();
}

int vtkOpenGLExtensionManager::ExtensionSupported(const char *name)
{
  this->Update();

  const char *p = this->ExtensionsString;
  int NameLen = strlen(name);

  while (1)
    {
    int n;
    while (*p == ' ') p++;
    if (*p == '\0')
      {
      return 0;
      }
    n = strcspn(p, " ");
    if ((NameLen == n) && (strncmp(name, p, n) == 0))
      {
      return 1;
      }
    p += n;
    }
  // Shutup compiler.
  return 0;
}

vtkOpenGLExtensionManagerFunctionPointer
    vtkOpenGLExtensionManager::GetProcAddress(const char *fname)
{
#ifdef VTK_USE_WGL_GET_PROC_ADDRESS
  return wglGetProcAddress(fname);
#endif //VTK_USE_WGL_GET_PROC_ADDRESS


#ifdef VTK_USE_APPLE_LOADER
  NSSymbol symbol = NULL;
  char *mangled_fname = new char[strlen(fname)+2];
  // Prepend a '_' to the function name.
  strcpy(mangled_fname+1, fname);
  mangled_fname[0] = '_';
  if (NSIsSymbolNameDefined(mangled_fname))
    {
    symbol = NSLookupAndBindSymbol(mangled_fname);
    }
  else
    {
    vtkWarningMacro("Could not load " << mangled_fname);
    }
  delete[] mangled_fname;
  if (symbol)
    {
    return (vtkOpenGLExtensionManagerFunctionPointer)NSAddressOfSymbol(symbol);
    }
  else
    {
    vtkWarningMacro("Could not load " << mangled_fname);
    return NULL;
    }
#endif //VTK_USE_APPLE_LOADER


#ifdef VTK_USE_GLX_GET_PROC_ADDRESS
  return glXGetProcAddress((const GLubyte *)fname);
#endif //VTK_USE_GLX_GET_PROC_ADDRESS
#ifdef VTK_USE_GLX_GET_PROC_ADDRESS_ARB
  return glXGetProcAddressARB((const GLubyte *)fname);
#endif //VTK_USE_GLX_GET_PROC_ADDRESS_ARB


#ifdef VTK_USE_VTK_DYNAMIC_LOADER
  // If the GLX implementation cannot load procedures for us, load them
  // directly from the dynamic libraries.
  static vtkstd::list<vtkstd::string> ogl_libraries;

  if (ogl_libraries.empty())
    {
    const char *ext = vtkDynamicLoader::LibExtension();
    vtkstd::string::size_type ext_size = strlen(ext);
    // Must be the first function we tried to load.  Fill this list with
    // the OpenGL libraries we linked against.
    vtkstd::string l(OPENGL_LIBRARIES);
    vtkstd::string::size_type filename_start = 0;
    while (1)
      {
      vtkstd::string::size_type filename_end = l.find(';', filename_start);
      if (filename_end == vtkstd::string::npos)
        {
        break;
        }
      vtkstd::string possible_file = l.substr(filename_start,
                                              filename_end-filename_start);
      // Make sure this is actually a library.  Do this by making sure it
      // has an appropriate extension.  This is by no means definitive, but
      // it1 should do.
      if (   (possible_file.length() > ext_size)
          && (possible_file.substr(possible_file.length()-ext_size) == ext) )
        {
        ogl_libraries.push_back(possible_file);
        }

      filename_start = filename_end + 1;
      }
    }

  // Look for the function in each library.
  for (vtkstd::list<vtkstd::string>::iterator i = ogl_libraries.begin();
       i != ogl_libraries.end(); i++)
    {
    vtkLibHandle lh = vtkDynamicLoader::OpenLibrary((*i).c_str());
    void *f = vtkDynamicLoader::GetSymbolAddress(lh, fname);
    vtkDynamicLoader::CloseLibrary(lh);
    if (f) return (vtkOpenGLExtensionManagerFunctionPointer)f;
    }

  // Could not find the function.
  return NULL;
#endif //VTK_USE_VTK_DYNAMIC_LOADER
}

void vtkOpenGLExtensionManager::LoadExtension(const char *name)
{
  if (!this->ExtensionSupported(name))
    {
    vtkWarningMacro("Attempting to load " << name
                    << ", which is not supported.");
    }

  int success = vtkgl::LoadExtension(name, this);

  if (!success)
    {
    vtkErrorMacro("Extension " << name << " could not be loaded.");
    }
}

static vtkstd::string VersionNumToExtensionsString(const char *ver_string,
                                                const char *type,
                                                vtkOpenGLExtensionManager *self)
{
  vtkstd::string extensions_string("");

  char *ver_string_copy = new char[strlen(ver_string)+1];
  strcpy(ver_string_copy, ver_string);

  char *num = strtok(ver_string_copy, ". ");
  int major_num = atoi(num);

  num = strtok(NULL, ". ");
  int minor_num = atoi(num);

  delete[] ver_string_copy;

  for (int major = 1; major <= major_num; major++)
    {
    // I can't think of a really good way to ensure we get exactly all minor
    // versions, but I doubt we'll ever need to deal with more than 10 minor
    // revisions.
    for (int minor = 0;
         (minor < 10) && ((major < major_num) || (minor <= minor_num)); minor++)
      {
      char extension_name[20];
      sprintf(extension_name, "%s_VERSION_%d_%d", type, major, minor);
      if (vtkgl::LoadExtension(extension_name, self))
        {
        extensions_string += " ";
        extensions_string += extension_name;
        }
      }
    }

  return extensions_string;
}

void vtkOpenGLExtensionManager::ReadOpenGLExtensions()
{
  vtkDebugMacro("ReadOpenGLExtensions");

  if (this->RenderWindow)
    {
    if (!this->RenderWindow->IsA("vtkOpenGLRenderWindow"))
      {
      // If the render window is not OpenGL, then it obviously has no
      // extensions.
      this->ExtensionsString = new char[1];
      this->ExtensionsString[0] = '\0';
      return;
      }
    if (this->RenderWindow->GetNeverRendered())
      {
      this->RenderWindow->Render();
      }
    this->RenderWindow->MakeCurrent();
    }

  vtkstd::string extensions_string;

  const char *gl_extensions;
  const char *glu_extensions = "";
  const char *win_extensions;

  gl_extensions = (const char *)glGetString(GL_EXTENSIONS);

  if (gl_extensions == NULL)
    {
    gl_extensions = "";
    }

  if (!this->RenderWindow && (gl_extensions[0] == '\0'))
    {
    vtkDebugMacro("No window active?  Attaching default render window.");
    vtkRenderWindow *renwin = vtkRenderWindow::New();
    renwin->SetSize(1, 1);
    this->SetRenderWindow(renwin);
    renwin->Delete();
    this->ReadOpenGLExtensions();
    return;
    }

  extensions_string = gl_extensions;

#if GLU_SUPPORTED
  glu_extensions = (const char *)gluGetString(GLU_EXTENSIONS);
#endif
  if (glu_extensions != NULL)
    {
    extensions_string += " ";
    extensions_string += glu_extensions;
    }

#if defined(WIN32)
  // Don't use this->LoadExtension or we will go into an infinite loop.
  vtkgl::LoadExtension("WGL_ARB_extensions_string", this);
  if (vtkwgl::GetExtensionsStringARB)
    {
    win_extensions = vtkwgl::GetExtensionsStringARB(wglGetCurrentDC());
    }
  else
    {
    vtkWarningMacro("Could not query WGL extensions.");
    win_extensions = "";
    }
#elif defined(__APPLE__)
//   vtkWarningMacro("Does APPLE have a windows extension string?");
  win_extensions = "";
#else
  win_extensions = glXGetClientString(glXGetCurrentDisplay(),
                                      GLX_EXTENSIONS);
#endif

  if (win_extensions != NULL)
    {
    extensions_string += " ";
    extensions_string += win_extensions;
    }

  extensions_string
    += VersionNumToExtensionsString((const char *)glGetString(GL_VERSION),
                                    "GL", this);
#if GLU_SUPPORTED
  extensions_string
    += VersionNumToExtensionsString((const char *)gluGetString(GLU_VERSION),
                                    "GLU", this);
#endif
#if !defined(WIN32) && !defined(__APPLE__)
  extensions_string
    += VersionNumToExtensionsString(glXGetClientString(glXGetCurrentDisplay(),
                                                       GLX_VERSION),
                                    "GLX", this);
#endif

  // Store extensions string.
  this->ExtensionsString = new char[extensions_string.length()+1];
  strcpy(this->ExtensionsString, extensions_string.c_str());
}