# Big mouse cursor pointer for partially sighted person (Windows Only)

Because version of Windows (lower than the 10th version) does not allow user to increase the size of the mouse pointer behind few pixel, it can be hard for partially sighted person to see the mouse pointer.

## How it works

The scritp set an hook over the mouse move to retrieve the its position on the screen and then the app moves a transparent window with a custom image (as png/jpg/bmp) with alpha channel supported over that position.

[img_how_it_works](!assets/how_it_works.png)
