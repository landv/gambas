Lighttable

This tool is written to view, sort and rename photographs.
For sorting, you can move the pictures freely around on the "table" like slides, that's where the name comes from.

Requirements:

exiftool, convert and md5sum must be installed (I think, they usually are).

Restrictions:

Right now, the program is written for .jp(e)g files only.
The program is written under KDE - no idea, if it is useable with GTK.

Recommendations:

1. Before starting, use Konqueror's preview (or Gwenview, Dolphin, Nautilus, Digikam or any similar program) to create thumbnails of your
   picture folder. They do that 10x times faster than I can. If this program has to create thumbnails, it takes almost 1 sec / picture.

2. For testing, don't choose a folder that contains hundreds of pictures. Loading takes time (but may be cancelled).

3. But while the pictures are loaded, you can already work. Just right-click on the background and on the frames to get context menus.

Questions:

1. How to use KDE's Thumbnail Generator (or any other of those fast programs) with a SHELL command?

2. When a file is renamed, the program also renames the thumbnail (in the folder /home/user/.thumbnails/normal). 
   Now it would be correct to also re-write the meta-info of the thumbnail. But no idea how to do that.

Have fun (I hope)!
Matti
math.eber@t-online.de
