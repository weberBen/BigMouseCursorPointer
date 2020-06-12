# Big mouse cursor pointer for partially sighted person (Windows Only)

Because version of Windows (lower than the 10th version) does not allow user to increase the size of the mouse pointer behind few pixel, it can be hard for partially sighted person to see the mouse pointer.
It can be seen as a far simpler version of *ZoomText* which is a costly solution for people who might only want to use the mouse pointer option.

## How it works

The scritp set an hook over the mouse move to retrieve the its position on the screen and then the app moves a transparent window with a custom image (as png/jpg/bmp) with alpha channel supported over that position.

<img src="/Assets/how_it_works.png" alt="How it works" width="450"/>

## Requirements

Windows over *Windows XP*

## Limitations

Since the app uses a window over the windows mouse pointer some system windows will not allow to set that window over them. In other words, for some system windows the custom mouse pointer will freeze (and then coma back to normal when the mouse exit the system window)

## In use

The app allows user to change and rescale the image while running


<img src="/Assets/scale_0.2.png" alt="Mouse pointer at scale 0.2" width="450"/>

<img src="/Assets/scale_0.5.png" alt="Mouse pointer at scale 0.5" width="450"/>
