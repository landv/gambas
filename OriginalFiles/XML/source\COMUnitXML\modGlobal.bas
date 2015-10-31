Attribute VB_Name = "modGlobal"
Option Explicit

Public Const g_Root = "COMUnitXML"
Public Const g_Version = 1.2

Public Enum g_Errors
    gblErrLoadXML = vbObjectError + 201
    gblErrMissingParametersNode = vbObjectError + 202
End Enum

Public Sub RaiseError(lngError As Long, strMessage As String)
    If (Err) Then
        Err.Raise lngError, App.Title, strMessage & vbNewLine & "Error (" & Err.Number & "): " & Err.Description
    Else
        Err.Raise lngError, App.Title, strMessage
    End If
End Sub

Public Function CreateXMLDocument() As DOMDocument
    Dim objDocument As New DOMDocument30
    
    Dim objRoot As IXMLDOMElement
    Set objRoot = objDocument.CreateElement("COMUnitXML")
    objRoot.setAttribute "version", g_Version
    Set objDocument.documentElement = objRoot
    
    Set CreateXMLDocument = objDocument
End Function

Public Function CreateElement(objDocument As DOMDocument, strName As String, strText As String) As IXMLDOMElement
    Dim objNode As IXMLDOMElement
    Set objNode = objDocument.CreateElement(strName)
    objNode.Text = strText
    Set CreateElement = objNode
End Function
