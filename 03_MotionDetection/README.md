# Chapter 03: Motion Detection - Learning Reflections

**Author**: Tony Fu  
**Date**: August 7, 2023

**Reference**: [Qt 5 and OpenCV 4 Computer Vision Projects](https://github.com/PacktPublishing/Qt-5-and-OpenCV-4-Computer-Vision-Projects/tree/master) by Zhuo Qingliang

## Results



## Core Concepts

- **Qt Layout system**:
    `QGridLayout` allow us to position widgets in a grid format, where we specify the row and column in which each widget will reside. Here is an example:

    1. First, create a new instance of `QGridLayout`.
    ```cpp
    QGridLayout *main_layout = new QGridLayout();
    ```

    2. Then, we can add widgets to specific grid positions using the `addWidget` method:
    ```cpp
    main_layout->addWidget(viewfinder, 0, 0, 12, 1);
    ```
    In this line, the `viewfinder` widget is placed at the top-left corner of the grid (row 0, column 0) and is set to span 12 rows and 1 column.

    3. If you want to, you can nest other layouts within it:
    ```cpp
    QGridLayout *tools_layout = new QGridLayout();
    main_layout->addLayout(tools_layout, 12, 0, 1, 1);
    ```
    Here, another `QGridLayout` named `tools_layout` is created and then added to the `main_layout`. This nested layout starts from the 13th row (index 12) and spans 1 row and 1 column.

    4. Inside `tools_layout`, widgets are added similarly:
    ```cpp
    tools_layout->addWidget(monitorCheckBox, 0, 0);
    tools_layout->addWidget(recordButton, 0, 1, Qt::AlignHCenter);
    ```

- **
