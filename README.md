# TheCarCV
This project was created to take part in competition of robots.
It can be trained to detect road signs and it can detect them.
It contains a bit of machine learning and works as fast as it is possible with hough circle transform of OpenCV. 

## Compiling and initial setup

1. Install the dependencies.
2. Run following commands in your terminal: 
    ```sh
    $ git clone https://github.com/IronProger/TheCar
    $ cd TheCar
    $ mkdir build
    $ cmake -B build
    $ cd build
    $ make
    ```
3. To check how well does it work go back to project's root directory and run the compiled binary:
    ```sh
    $ cd ..
    $ buid/TheCar
    ```
    Maybe you need to setup video input source number by changing **/configuration/video/video_input_source_number** in **config.xml**
    
## Dependencies

* [OpenCV — Open source computer vision](http://opencv.org)
* [plog — Portable, simple and extensible C++ logging library](https://github.com/SergiusTheBest/plog)
* [pugixml — Light-weight, simple and fast XML parser for C++ with XPath support ](https://github.com/zeux/pugixml)

Note: OpenCV and pugixml can be in repositories of your linux distributive.

## Quick start

1. Unpack quick_start.zip to project's root directory.
2. Set **mode** to **detection** in **config.xml**.
3. Enjoy.

## How to use

First simple start it and check how it works. If all are fine, go by following steps:
1. Set **mode** to **collect** in **config.xml**.
2. Start the program and make shots of road signs you need.
3. Manually move good shots of road signs to directories named by road signs names
(this will be directories **./ml_data/originals/*** and they will be created automatically
when the program is in collect mode).
4. Set **mode** to **learning** in **config.xml** and run the program.
5. See the log. If models is created successfully, set **mode** to **detection**.
6. Enjoy.

### About modes of working (from config.xml)

| mode | description |
| --- | --- |
| video | show video stream, mark circles which contain minimal value of blue color |
| collect | find circles which contain minimal value of blue color and save it ti disk |
| quiet_collect | the same mode without GUI |
| learning | make models using collected images |
| detection | detect road signs, output the result to the window and print to a log file |
| quiet_detection | the same mode without GUI |

## A&Q

**Is it possible to detect anything other than road signs?**

Yes, of course. To have done it please read «How to use».

**Why do I see an image with an increased value of blue and a reduced value of red?**

Because the program didn't work well with red. Now the program sends a monochrome image
for recognition, in which white is blue, and black is all other colors.
So red is not important now.

**Why does it detect badly road sign STOP?**

The program find circles only but road sign STOP is hexagon.

**Is there used machine learning?**

Partly. There is a partly realisation of convolutional neural network (one convolution
layer with manually set weights, pooling and ReLU). Recognition occurs by comparing the results
of this CNN for different images using standard derivation: the average model of a road sign
and the current shot.

## Todos

* Add finding for hexagons on video stream  (for the road sign STOP).
* Add traffic light detection.
* Add serial connection support with Arduino.
* Add FPS output to the window.
* Write CONFIGURATION.md.

## License

The TheCarCV is open-source software licensed under the [MIT license](https://opensource.org/licenses/MIT).
