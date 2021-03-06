/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkXMLDataElement.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkXMLDataElement - Represents an XML element and those nested inside.
// .SECTION Description
// vtkXMLDataElement is used by vtkXMLDataParser to represent an XML
// element.  It provides methods to access the element's attributes
// and nested elements in a convenient manner.  This allows easy
// traversal of an input XML file by vtkXMLReader and its subclasses.

// .SECTION See Also
// vtkXMLDataParser

#ifndef __vtkXMLDataElement_h
#define __vtkXMLDataElement_h

#include "vtkObject.h"

class vtkXMLDataParser;

class VTK_IO_EXPORT vtkXMLDataElement : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkXMLDataElement,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkXMLDataElement* New();
  
  // Description:
  // Set/Get the name of the element.  This is its XML tag.
  vtkGetStringMacro(Name);
  vtkSetStringMacro(Name);
  
  // Description:
  // Set/Get the value of the id attribute of the element, if any.
  vtkGetStringMacro(Id);
  vtkSetStringMacro(Id);
  
  // Description:
  // Get the attribute with the given name.  If it doesn't exist,
  // returns 0.
  const char* GetAttribute(const char* name);

  // Description:
  // Set the attribute with the given name and value. If it doesn't exist,
  // adds it.
  void SetAttribute(const char* name, const char* value);
 
  // Description:
  // Set/Get the character data between XML start/end tags.
  void SetCharacterData(const char* c, int length);
  void AddCharacterData(const char* c, int length);
  vtkGetStringMacro(CharacterData);

  // Description:
  // Get the attribute with the given name and converted to a scalar
  // value.  Returns whether value was extracted.
  int GetScalarAttribute(const char* name, int& value);
  int GetScalarAttribute(const char* name, float& value);
  int GetScalarAttribute(const char* name, double& value);
  int GetScalarAttribute(const char* name, unsigned long& value);

  // Description:
  // Set the attribute with the given name.
  // We can not use the same GetScalarAttribute() construct since
  // the compiler will not be able to resolve between
  // SetAttribute(..., int) and SetAttribute(..., unsigned long).
  void SetIntAttribute(const char* name, int value);
  void SetFloatAttribute(const char* name, float value);
  void SetDoubleAttribute(const char* name, double value);
  void SetUnsignedLongAttribute(const char* name, unsigned long value);
  
  // Description:
  // Get the attribute with the given name and converted to a scalar
  // value.  Returns length of vector read.
  int GetVectorAttribute(const char* name, int length, int* value);
  int GetVectorAttribute(const char* name, int length, float* value);
  int GetVectorAttribute(const char* name, int length, double* value);
  int GetVectorAttribute(const char* name, int length, unsigned long* value);
  
  // Description:
  // Set the attribute with the given name.
  void SetVectorAttribute(const char* name, int length, const int* value);
  void SetVectorAttribute(const char* name, int length, const float* value);
  void SetVectorAttribute(const char* name, int length, const double* value);
  void SetVectorAttribute(const char* name, int length, const unsigned long* value);

#ifdef VTK_USE_64BIT_IDS
  //BTX
  int  GetScalarAttribute(const char* name, vtkIdType& value);
  void SetIdTypeAttribute(const char* name, vtkIdType value);
  int  GetVectorAttribute(const char* name, int length, vtkIdType* value);
  void SetVectorAttribute(const char* name, int length, const vtkIdType* value);
  //ETX
#endif
  
  // Description:
  // Get the attribute with the given name and converted to a word type
  // such as VTK_FLOAT or VTK_UNSIGNED_LONG.
  int GetWordTypeAttribute(const char* name, int& value);
  
  // Description:
  // Get the number of attributes.
  vtkGetMacro(NumberOfAttributes, int);

  // Description:
  // Get the n-th attribute name.
  // Returns 0 if there is no such attribute.
  const char* GetAttributeName(int idx);

  // Description:
  // Get the n-th attribute value.
  // Returns 0 if there is no such attribute.
  const char* GetAttributeValue(int idx);

  // Description:
  // Remove all attributes.
  virtual void RemoveAllAttributes();

  // Description:
  // Set/Get the parent of this element.
  vtkXMLDataElement* GetParent();
  void SetParent(vtkXMLDataElement* parent);
  
  // Description:
  // Get root of the XML tree this element is part of.
  virtual vtkXMLDataElement* GetRoot();
  
  // Description:
  // Get the number of elements nested in this one.
  int GetNumberOfNestedElements();
  
  // Description:
  // Get the element nested in this one at the given index.
  vtkXMLDataElement* GetNestedElement(int index);
  
  // Description:
  // Add nested element
  void AddNestedElement(vtkXMLDataElement* element);

  // Description:
  // Remove nested element.
  virtual void RemoveNestedElement(vtkXMLDataElement *);
  
  // Description:
  // Remove all nested elements.
  virtual void RemoveAllNestedElements();

  // Description:
  // Find a nested element with the given id, given name, or given name and id.
  vtkXMLDataElement* FindNestedElement(const char* id);
  vtkXMLDataElement* FindNestedElementWithName(const char* name);
  vtkXMLDataElement* FindNestedElementWithNameAndId(
    const char* name, const char* id);
  vtkXMLDataElement* FindNestedElementWithNameAndAttribute(
    const char* name, const char* att_name, const char* att_value);
  
  // Description:
  // Lookup the element with the given id, starting at this scope.
  vtkXMLDataElement* LookupElement(const char* id);
  
  // Description:
  // Set/Get the offset from the beginning of the XML document to this element.
  vtkGetMacro(XMLByteIndex, unsigned long);
  vtkSetMacro(XMLByteIndex, unsigned long);
  
  // Description:
  // Check if the instance has the same name, attributes, character data
  // and nested elements contents than the given element (this method is
  // applied recursively on the nested elements, and they must be stored in
  // the same order).
  // Warning: Id, Parent, XMLByteIndex are ignored.
  virtual int IsEqualTo(vtkXMLDataElement *elem);

  // Description:
  // Copy this element from another of the same type (elem), recursively.
  // Old attributes and nested elements are removed, new ones are created
  // given the contents of 'elem'.
  // Warning: Parent is ignored.
  virtual void DeepCopy(vtkXMLDataElement *elem);

  // Description:
  // Get/Set the internal character encoding of the attributes.
  // Default type is VTK_ENCODING_UTF_8.
  // Note that a vtkXMLDataParser has its own AttributesEncoding ivar. If 
  // this ivar is set to something other than VTK_ENCODING_NONE, it will be
  // used to set the attribute encoding of each vtkXMLDataElement 
  // created by this vtkXMLDataParser.
  vtkSetClampMacro(AttributeEncoding,int,VTK_ENCODING_NONE,VTK_ENCODING_UNKNOWN);
  vtkGetMacro(AttributeEncoding, int);
  
protected:
  vtkXMLDataElement();
  ~vtkXMLDataElement();  
  
  // The name of the element from the XML file.
  char* Name;
  
  // The value of the "id" attribute, if any was given.
  char* Id;
  
  // The character data between the start and end tags of this element.
  char* CharacterData;
  
  // The offset into the XML stream where the element begins.
  unsigned long XMLByteIndex;
  
  // The offset into the XML stream where the inline data begins.
  unsigned long InlineDataPosition;
  
  // The raw property name/value pairs read from the XML attributes.
  char** AttributeNames;
  char** AttributeValues;
  int NumberOfAttributes;
  int AttributesSize;
  int AttributeEncoding;

  // The set of nested elements.
  int NumberOfNestedElements;
  int NestedElementsSize;
  vtkXMLDataElement** NestedElements;
  
  // The parent of this element.
  vtkXMLDataElement* Parent;
  
  // Method used by vtkXMLFileParser to setup the element.
  void ReadXMLAttributes(const char** atts, int encoding);  
  void SeekInlineDataPosition(vtkXMLDataParser* parser);
  
  void PrintXML(ostream& os, vtkIndent indent);
  
  // Internal utility methods.
  vtkXMLDataElement* LookupElementInScope(const char* id);
  vtkXMLDataElement* LookupElementUpScope(const char* id);
  static int IsSpace(char c);
  
  //BTX
  friend class vtkXMLDataParser;
  friend class vtkXMLMaterialParser;
  //ETX

private:
  vtkXMLDataElement(const vtkXMLDataElement&);  // Not implemented.
  void operator=(const vtkXMLDataElement&);  // Not implemented.
};

#endif
