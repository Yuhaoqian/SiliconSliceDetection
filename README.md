### 1 Overview
- This project aims at detecting the silicon slice, deciding whether the slice, over conveyor belt in factory, accords with the production specification(C++, Opencv)
- In order to process the image we import OpenCv lib.

### 2 Configure opencv environment in Xcode
1. use homebrew to install opencv
brew install opencv
2. Find 'Header Search Path' in 'Buid Setting' in Xcode, then add the head file link to there /usr/local/Cellar/opencv/4.2.0_1/include/opencv4
3. Find 'Linked Frameworks abd Libraries' in 'General', add *.dylib files to there in /usr/local/Cellar/opencv/4.2.0_1/lib 
