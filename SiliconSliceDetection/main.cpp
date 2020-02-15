#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;

Mat src, dst, src_gray;
Mat m;

int main() {
    // Read image
    string root = "../images/";
    string directoryName = "FrontError";
    string fileName = "03.bmp";
    string fileLocation = root + directoryName + '/' + fileName;
    cout << fileLocation << endl;
    src = imread(fileLocation);
    if (src.empty()) {
        cout << "no";
        return -1;
    }
    
    namedWindow("origin", 0); // Show the original image - 0 means the window can be dragged
    imshow("origin", src);
    
    waitKey(0);
    return 0;
}
    

