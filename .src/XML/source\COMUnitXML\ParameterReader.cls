VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "ParameterReader"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit

Public Function ReadFromFile(strFilename As String) As TestParameters
    Dim objDocument As New DOMDocument30
    objDocument.Load strFilename
    If (objDocument.documentElement Is Nothing) Then
        Err.Raise g_Errors.gblErrLoadXML, "COMUnitXML.ParameterReader", "Failed to load XML file."
    End If
    
    ' find root node
    Dim objNodeList As IXMLDOMNodeList
    Set objNodeList = objDocument.getElementsByTagName("parameters")
    
    If (objNodeList.length = 0) Then
        Err.Raise g_Errors.gblErrMissingParametersNode, "COMUnitXML.ParameterReader", "Document does not contain Parameters node"
    End If
    
    Set ReadFromFile = ReadFromNode(objNodeList(0))
End Function

Public Function ReadFromNode(objNode As IXMLDOMElement) As TestParameters
    Dim objParameters As New TestParameters
            
    Dim objChild As IXMLDOMNode
    For Each objChild In objNode.childNodes
        If (objChild.nodeName = "parameter") Then
            objParameters.addParameter ReadParameter(objChild)
        End If
    Next
    
    Set ReadFromNode = objParameters
End Function

Public Function ReadParameter(objNode As IXMLDOMNode) As TestParameter
    Dim objParameter As New TestParameter
    
    Dim objChild As IXMLDOMNode
    For Each objChild In objNode.childNodes
        If (objChild.nodeName = "name") Then
            objParameter.Name = objChild.Text
        ElseIf (objChild.nodeName = "value") Then
            objParameter.Value = objChild.Text
        ElseIf (objChild.nodeName = "parameter") Then
            objParameter.Parameters.addParameter ReadParameter(objChild)
        End If
    Next
    
    Set ReadParameter = objParameter
End Function
