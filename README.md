About
=====

Caméléon is a program that lets you browse synchronously multiple collections of images. 

For instance, if you are developing a machine-learning algorithm for image segmentation, you might want to compare input images, their ground-truth segmentations, and the segmentations produced by your algorithm. In this scenario, Caméléon will let you view the three images corresponding to each instance next to each other, pan and zoom them simultaneously, and switch easily between different instances.

Or if you are running numerical simulations of multiple systems differing by the value of some parameter, Caméléon will let you view maps of all the fields of interest (for instance, velocity and pressure, stress and strain, or electric and magnetic field) in a given system and switch quickly between results obtained for different systems.

Quick start
===========

Consider the first scenario described above: you have trained a machine-learning model for image segmentation. You have tested your model on some instances and generated a folder `C:\ML\Model-1` containing the files `input-<n>.png`, `ground-truth-<n>.png` and `prediction-<n>.png` for `n` ranging from 1 to the number of instances. Now you would like to browse the segmentations produced by the model and compare them to the ground-truth segmentations and the input images.

Start Caméléon and select the *Album | New...* menu item. A _New Album_ dialog will appear; in the top three combo boxes, enter wildcard patterns representing the paths to the images to be displayed on each page of the album, `C:\ML\Model-1\input-*.png`, `C:\ML\Model-1\ground-truth-*.png`, and `C:\ML\Model-1\prediction-*.png`:

![New Album dialog](/doc/images/new-album.png)

Click OK. The first page of the album, corresponding to instance `1`, will be displayed:

![First page of the album](/doc/images/album.png)

You can zoom in and out using the *View | Zoom in/Zoom out* menu items or pressing the `Ctrl` button, placing the mouse pointer on one of the displayed images and scrolling the wheel up or down. All images will be zoomed and scrolled together:

![Zoom](/doc/images/zoom.png)

To switch between pages, use the *Navigation* menu, the four toolbar buttons highlighted in red in the screenshot below, or the `Home`, `PgDn`, `PgUp` and `End` keyboard shortcuts. You can also navigate directly to a specific page by selecting its ID (made up of the matches to all wildcards) from the pull-down list highlighted in green in the screenshot below:

![Navigation controls](/doc/images/navigation.png)

You can change the album layout by selecting an appropriate item from the *View | Layout* submenu. For example, click *View | Layout | 3x1* to switch to a layout with one column and three rows:

![Alternative layout](/doc/images/layout.png)

By default, each image is labelled with its path. This may be difficult to read, especially if the path needs to be truncated to fit on the screen. Click *View | Edit Captions...* to set a custom caption for each panel:

![Edit Captions dialog](/doc/images/edit-captions.png)

The new captions are visible in the screenshot below:

![Edited captions](/doc/images/edited-captions.png)

To save a screenshot of the current page, click *View | Save Screenshot...* and select the path where the screenshot should be saved. The default file name is derived from the current page ID.

Click *Album | Save* to save the current album and *Album | Open...* to open one saved previously.

> [!NOTE]
> The following wildcards are supported:
> * `*`: matches any number of characters in a single file or folder name (in other words, it does not match directory separators `/` or `\`)
  * `**` matches any number of complete file and folder names (e.g. `a/**/z` will match `a/z`, `a/b/z`, `a/b/c/z` etc.)
  * `?` matches any single character.
> All wildcard patterns must contain the same number of wildcards. Exception: a pattern may contain no wildcards even if other patterns do; in this case the panel corresponding to that pattern will display the same image on all pages.

Installation
============

Download a ZIP or TGZ archive with a pre-built version of Caméléon from https://github.com/wsmigaj/Cameleon/releases. Uncompress the archive to a folder of your choice.

Building
========

Install a C++ compiler, CMake 3.19 or later and Qt 6.2 (later versions may work too, but have not been tested).

Create a build folder, `cd` into it and initialize it by running CMake with the `CMAKE_PREFIX_PATH` variable set to Qt's installation prefix, e.g. `C:\Qt\6.2.3\msvc2019_64`:

    cmake <path-to-source-folder> -DCMAKE_PREFIX_PATH=<qt-installation-prefix>

Build the code:

    cmake --build .

Install the application:

    cmake --install . --prefix <installation-folder>

> [!NOTE]
> If you are using a multi-configuration CMake generator (e.g. the Visual Studio generator), append `--config Release` or `--config Debug` to the above two commands to select the desired configuration. Otherwise append `-DCMAKE_BUILD_TYPE=Release` or `-DCMAKE_BUILD_TYPE=Debug` to the command initializing the build folder.

Source code formatting
======================

1. Install the `pre-commit` Python package (possibly in a virtual Python environment):

       pip install pre-commit
       
2. Run the following command in the top-level directory of the Git repository to install a pre-commit hook using `clang-format` to format C++ sources:

       pre-commit install
