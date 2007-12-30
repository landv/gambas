
Printing in Gambas

This example project is designed to show you how to perform simple printing of
text, images and drawings. There are comments in the code that describe what we
are doing. Here I just want to provide some basic background information.

The application itself is divided into two sections. The first is the main form
with a TabStrip on it. Each tab has a button that allows you to open a file(s) in
a particular format: text, image or drawing.  After you open the file(s) you can
use the print button.

The most important section are the modules that contains our printer procedures. I
have placed these procedures in separate modules to make it easy to add simple
printing to your Gambas project. Simply take a copy of the required module file
and drop it into the directory of your Gambas project. Open your project and
call these printing function. How to call these functions is described in each
module.

Printing Text:

  PUBLIC SUB PrintText(Text AS String, TextFont AS Font, TextColor AS Integer, OPTIONAL WrapText AS Boolean)

In Gambas the Printer object is a graphics device. So can use the methods of
the Draw class to draw on the printer in the same way you can draw on a
Window, Picture, Drawing or DrawingArea. 

For printing text we split our text into lines and then print each line using
the supplied font and color. We need the height of a line of text in order to
know how much further down the page we need to move for each line. The Draw
object provides the TextHeight method for this. Before you call this method you
must have called the printer setup dialog and set the required font for
printing text. The printer resolution and the font affect the value returned
for the text height.

The final option sets the word wrap option. The Gambas Draw class does not include
a word wrap option. We have added a simple function to wrap text onto the next line
given a maximum width. Our function does not attempt to hyphen words. But it will
split the first word of a line if it is longer than the line width.

Printing Images

  PUBLIC SUB PrintImage(img AS Image, OPTIONAL FitToPage AS Boolean)

Computer image formats come in two types: vector graphics and raster graphics.
Vector graphics file formats store a description of how to construct the image.
These formats can be easily scaled. In Gambas you  use the Drawing object which
supports the SVG format (see below). Raster graphics describe the color of each
pixel in the image. There is also often some form of data compression to reduce
the size of the file. In Gambas these formats are supported by the Picture and
Image objects. Picture and Image objects supports the following file formats:

  Portable Network Graphics (*.png)
  Joint Photographic Experts Group (*.jpeg *.jpg)
  Windows Bitmap (*.bmp)
  Graphics Interchange Format (*.gif)
  X PixMap (*.xpm)

It is usually easy to convert a vector graphics drawing to a  raster graphics
images. However it is often very complex to try to convert images to drawings.

The most important issue to deal with when printing images is to scale them
for the printer. The resolution on the screen is different to that on the
printer. On my system the Desktop resolution returns 75 DPI and the Printer
resolution returns 600 DPI. These are fairly typical values. But you should
not rely on these values if you want your application to be used on as many
Linux systems as possible.

Any resizing of the image should be done after you have called the printer
setup dialog. If the user has more than one printer attached to their
workstation then they may swap printers in this dialog or swap to printing to
a file. By calling the printer setup dialog first you ensure the values you are
using are valid for the printer the user selected.

If you have a Picture object instead of an Image it is simple to call this
procedure. You could call the function with:

  ModulePrintImage.PrintImage(displayPicture.Image)

We need to use an Image in the procedure because you can not Stretch a Picture.

Printing Drawings

  PUBLIC SUB PrintDrawing(drw AS Drawing, OPTIONAL FitToPage AS Boolean)

Scalable Vector Graphics (SVG) are becoming an increasingly popular standard
for drawings. These files store there drawing description in XML format. You
can look at this markup by simply opening the SVG file in any text editor
(try it in the text tab on this demo). Most vector drawing applications support
this standard and Gambas has basic support for it also. The web site
http://openclipart.org/index.php is a useful source of clip art, much of it in
this format.
 
Printing a drawing is very similar to printing a image. The Printer dialog code
and the scaling code to find the required size for the drawing is almost
exactly the same. Then the Draw.Drawing method will perform the resizing for
you.

Printer Layout

The Two Images tab demonstrates how you can mix the printing of text and images.
There is nothing in the code that we have not seen already. There two buttons for
opening images one for the top image and the other for the bottom. Also there are
two printing buttons.

The first printing button will layout the text and images as follows:

  Top Text
  Top Image
  Bottom Text
  Bottom Image

The text will be centred and places with a box. The second printing button will
layout the text as follows:

  Top Text – Top Image
  Bottom Image – Bottom Text 

the text will be wrapped along the side of the page. 

Timothy Marshal-Nichols
August 2006
timothy@m-nichols.freeserve.co.uk
