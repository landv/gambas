VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "ParameterWriter"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

Public Function WriteToFile(objParameters As TestParameters, strFilename As String) As DOMDocument
    Dim objDocument As DOMDocument30
    Set objDocument = WriteToDocument(objParameters)
    
    objDocument.save strFilename
    
    Set WriteToFile = objDocument
End Function

Public Function WriteToDocument(objParameters As TestParameters) As DOMDocument
    Dim objDocument As DOMDocument
    Set objDocument = CreateXMLDocument
    objDocument.documentElement.appendChild WriteToNode(objDocument, objParameters)
    
    Set WriteToDocument = objDocument
End Function

Public Function WriteToNode(objDocument As DOMDocument, objParameters As TestParameters) As IXMLDOMElement
    Dim objRoot As IXMLDOMElement
    Set objRoot = objDocument.CreateElement("parameters")
    
    Dim objParameter As TestParameter
    For Each objParameter In objParameters
        objRoot.appendChild CreateParameterNode(objDocument, objParameter)
    Next
    
    Set WriteToNode = objRoot
End Function

Private Function CreateParameterNode(objDocument As DOMDocument, objParameter As TestParameter) As IXMLDOMElement
    Dim objNode As IXMLDOMElement
    Set objNode = objDocument.CreateElement("parameter")
    
    Dim objNameNode As IXMLDOMElement
    Set objNameNode = objDocument.CreateElement("name")
    objNameNode.Text = objParameter.Name
    objNode.appendChild objNameNode
    
    Dim objValueNode As IXMLDOMElement
    Set objValueNode = objDocument.CreateElement("value")
    objValueNode.Text = objParameter.Value
    objNode.appendChild objValueNode
    
    Dim objSubParameter As TestParameter
    For Each objSubParameter In objParameter.Parameters
        objNode.appendChild CreateParameterNode(objDocument, objSubParameter)
    Next
    
    Set CreateParameterNode = objNode
End Function
