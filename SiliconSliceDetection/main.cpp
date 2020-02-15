#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;

// global variables
#define WIDTH 640 // width of the image
#define HEIGHT 480 // height of the image
#define LENGTH 446 // the measurement of the silicon slice
#define DEPTH 5 // defect depth
#define LERROR 15 // the range of length difference

int leftEdge; // the left start point of the img
int rightEdge; // the right start point of the img
int topLength = 0, bottomLength = 0, leftLength = 0, rightLength = 0;

Mat src, dst, src_gray;
Mat m;
int vertex[4][2] = {0}; // top->right->bottom->left

void getEdge(Mat img);
void calLength();
int checkLength();

int posDetection(Mat img);
void edgeDetection(Mat img, int pos);
void vertexDetection(Mat img, int pos);
int allDetection(Mat img, int pos); 
// determine if the slice is valid. 
// If it has problems, return the error type. 
// 1->left and right length don't match 
// 2->top and bottom length don't match 
// 3->right edge has problems
// 4->bottom edge has problems
// 5->left edge has problems
// 6->top edge has problems
// 0->the slice is normal

int posDetection(Mat img) { // Check the position of the silicon slice
    int startImgRow = 0;    // the row that the the image's top boundry is on
    int endImgRow = HEIGHT - 1; // the row that the image's bottom boundry is on
    int middleImgRow = HEIGHT / 2; // the row that the image's middle is on
    int middleImgCol = WIDTH / 2; // the column that the image's middle is on

    //  use flags to decide whether the silicon slice can be seen from the top, middle or bottom by the camera
    int topFlag = 0;
    int middleFlag = 0;
    int bottomFlag = 0; 

    // detect the top section
    if ((img.at<uchar>(startImgRow, WIDTH / 2 - 1) == 0) 
            && (img.at<uchar>(startImgRow, WIDTH / 2) == 0)
            && (img.at<uchar>(startImgRow, WIDTH / 2 + 1) == 0)){
        topFlag = 1;
    }
    // detect the middle section
    if ((img.at<uchar>(middleImgRow, WIDTH / 2 - 1) == 0) 
            && (img.at<uchar>(middleImgRow, WIDTH / 2) == 0) 
            && (img.at<uchar>(middleImgRow, WIDTH / 2 + 1) == 0)) {
        middleFlag = 1;
    }
    // detect the bottom section
    if ((img.at<uchar>(endImgRow, WIDTH / 2 - 1) == 0) 
            && (img.at<uchar>(endImgRow, WIDTH / 2) == 0)
            && (img.at<uchar>(endImgRow, WIDTH / 2 + 1) == 0)) {
        bottomFlag = 1;
    }
    if (topFlag == 1) {
        if (middleFlag == 0) {
            return 1; // 1 0
        } else {
            return 2; // 1 1
        }
    } else { // top = 0
        if (middleFlag == 1) {
            if (bottomFlag == 0) {
                return 3; // 0 1 0
            } else {
                return 4; // 0 1 1
            }
        } else {
            if (bottomFlag == 1) {
                return 5; // 0 0 1
            } else {
                return 6; // 0 0 0
            }
        }
    }
}
void getEdge(Mat img) {
    for (int j = 1; j < WIDTH / 2; j++) {
        if (img.at<uchar>(HEIGHT / 2, j - 1) == 0
                && img.at<uchar>(HEIGHT / 2, j) == 250
                && img.at<uchar>(HEIGHT / 2, j + 1) == 250) {
            leftEdge = j + 1;
            break;
        }
    }
    for (int j = WIDTH - 2; j > 1; j--) {
        if (img.at<uchar>(HEIGHT / 2, j + 1) == 0 
                && img.at<uchar>(HEIGHT / 2, j) == 250
                && img.at<uchar>(HEIGHT / 2, j - 1) == 250) {
            rightEdge = j - 1;
            break;
        }
    }
    leftEdge += 10;
    rightEdge -= 10;
}

void edgeDetection(Mat img, int pos) {
    if (pos == 1) {
            } else if (pos == 2) {
    } else if (pos == 3) {
    }
}
void vertexDetection(Mat img, int pos) {
    if (pos == 1) {
        // check the left top vertex
        for (int j = leftEdge; j < rightEdge; j++) {
            if (img.at<uchar>(0, j - 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[0][0] = 0;
                vertex[0][1] = j;
                break;
            }
        }
        // check the right top vertex
        for (int j = rightEdge - 1; j > leftEdge; j--) {
            if (img.at<uchar>(0, j + 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[1][0] = 0;
                vertex[1][1] = j;
                break;
            }
        }
        // check the bottom vertex
        int endFor = 0;
        for (int i = HEIGHT / 2; i > 4 && endFor == 0; i--) {
            for (int j = rightEdge; j > leftEdge; j--) {
                if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                    if (j > WIDTH / 2) { //  bottom vertex is on right half, which is the regular condition
                        while (img.at<uchar>(i - 1, j) == 0) j++;
                        while (img.at<uchar>(i - 2, j) == 0) j++;
                        while (img.at<uchar>(i - 3, j) == 0) j++;
                        vertex[2][0] = i;
                        vertex[2][1] = j;
                        endFor = 1; // find the bottom vertex, finish

                       break;
                    } else { // bottom vertex is on the right half, so the vertex is left bottom vertex
                        while (img.at<uchar>(i, j) == 0) j--;
                        while (img.at<uchar>(i - 1, j) == 0) j--;
                        while (img.at<uchar>(i - 2, j) == 0) j--;
                        while (img.at<uchar>(i - 3, j) == 0) j--;
                        vertex[3][0] = i;
                        vextex[3][0] = j;
                        for (int k = rightEdge; k > WIDTH / 2; k--) {
                            for (int l = HEIGHT / 2; l > 1; l--) {
                                if (img.at<uchar>(l + 1, k) == 250 && img.at<uchar>(l, k) == 0) {
                                    while (img.at<uchar>(l, k - 1) == 0) l++;
                                    while (img.at<uchar>(l, k - 2) == 0) l++;
                                    while (img.at<uchar>(l, k - 3) == 0) l++;

                                    vertex[2][0] = l;
                                    vertex[2][1] = k;
                                    return;
                                }
                            }
                        }
                        endFor = 2;
                    }
                }
            }
        }
        if (endFor == 1) {
            for (int j = leftEdge; j < WIDTH / 2; j++) {
                for (int i = HEIGHT / 2; i > 1; i--) {
                    if (img.at<uchar>(i + 1, j) == 250 img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j + 1) == 0) i++;
                        while (img.at<uchar>(i, j + 2) == 0) i++;
                        while (img.at<uchar>(i, j + 3) == 0) i++;
                        vertex[3][0] = i;
                        vertex[3][1] = j;
                        return;
                    }
                }
            }
        }
    } else if (pos == 2) {
         // check the left top vertex
        for (int j = leftEdge; j < rightEdge; j++) {
            if (img.at<uchar>(0, j - 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[0][0] = 0;
                vertex[0][1] = j;
                break;
            }
        }
        // check the right top vertex
        for (int j = rightEdge - 1; j > leftEdge; j--) {
            if (img.at<uchar>(0, j + 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[1][0] = 0;
                vertex[1][1] = j;
                break;
            }
        }
        // check the bottom vertex
        int endFor = 0;
        for (int i = HEIGHT - 5; i > HEIGHT / 2 && endFor == 0; i--) {

            for (int i = rightEdge; j > leftEdge; j--) {
                if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                    if (j > WIDTH / 2) {
                        while (img.at<uchar>(i - 1, j) == 0) j++;
                        while (img.at<uchar>(i - 2, j) == 0) j++;
                        while (img.at<uchar>(i - 3, j) == 0) j++;

                        vertex[2][0] = i;
                        vertex[2][1] = j;
                        endFor = 1;
                        break;
                    } else {
                        while (img.at<uchar>(i, j) == 0) j--;
                        while (img.at<uchar>(i - 1, j) == 0) j--;
                        while (img.at<uchar>(i - 2, j) == 0) j--;
                        while (img.at<uchar>(i - 3, j) == 0) j--;
                        vertex[3][0] = i;
                        vertex[3][1] = j;
                        for (int k = rightEdge; k > WIDTH / 2; k--) {
                            for (int l = HEIGHT - 2; l > HEIGHT / 2; l--) {
                                if (img.at<uchar>(l + 1, k) == 250 && img.at<uchar>(l, k) == 0) {
                                    while (img.at<uchar>(l, k - 1) == 0) l++;
                                    while (img.at<uchar>(l, k - 2) == 0) l++;
                                    while (img.at<uchar>(l, k - 3) == 0) l++;
                                    vertex[2][0] = l;
                                    vertex[2][1] = k;
                                    return;
                                }
                            }
                        }
                        endFor = 2;
                    }
                }
            }
        }
        if (endFor == 1) {
            for (int j = leftEdge; j < WIDTH  / 2; j++) {
                for (int i = HEIGHT - 5; i >HEIGHT / 2; i--) {
                    if (img.at<uchar>(i + 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j + 1) == 0) i++;
                        while (img.at<uchar>(i, j + 2) == 0) i++;
                        while (img.at<uchar>(i, j + 3) == 0) i++;
                        vertex[3][0] = i;
                        vertex[3][1] = j;
                        return;
                    }
                }
            }
        }
    } else if (pos == 3) {
        int endFor = 0;
        // detect the left/right vertex
        for (int i = 1; i < HEIGH / 2 && endFor == 0; i++) {
            for (int j = leftEdge; j < rightEdge; j++) {
                if (img.at<uchar>(i, j - 1) == 250 && img.at<uchar>(i, j) == 0) {
                    if (j > WIDTH / 2) { // right vertex
                        while (img.at<uchar>(i, j) == 0) j++;
                        while (img.at<uchar>(i + 1, j) == 0) j++;
                        while (img.at<uchar>(i + 2, j) == 0) j++;
                        while (img.at<uchar>(i + 3, j) == 0) j++;
                        while (img.at<uchar>(i + 4, j) == 0) j++;
                        vertex[1][0] = i;
                        vertex[1][1] = j;
                        endFor = 1;
                        break;
                    } else {
                        while (img.at<uchar>(i + 1, j) == 0)j--;
                        while (img.at<uchar>(i + 2, j) == 0)j--;
                        while (img.at<uchar>(i + 3, j) == 0)j--;
                        while (img.at<uchar>(i + 4, j) == 0)j--;
                        ver[0][0] = i;
                        ver[0][1] = j;
                        endFor = 2;
                        break;
                    }
                }
            }
        }
        if (endFor == 1) {// lost the left top vertex
            for (int j = leftEdge; j < WIDTH / 2 && endFor == 1; j++) {
                for (int i = 1; i < HEIGHT / 2; i++) {
                    if (img.at<uchar>(i - 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j + 1) == 0) { i--; }
                        while (img.at<uchar>(i, j + 2) == 0) { i--; }
                        while (img.at<uchar>(i, j + 3) == 0) { i--; }
                        while (img.at<uchar>(i, j + 4) == 0) { i--; }
                        ver[0][0] = i;
                        ver[0][1] = j;
                        endFor = 3; // anticlockwise rotation
                        break;
                    }
                }
            }
        }
        if (endFor == 2) {// lost the right top vertex
            for (int j = rightEdge; j > WIDTH / 2 && endFor == 2; j--) {
                for (int i = 1; i < HEIGHT / 2; i++) {
                    if (img.at<uchar>(i - 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j - 1) == 0)i--;
                        while (img.at<uchar>(i, j - 2) == 0)i--;
                        while (img.at<uchar>(i, j - 3) == 0)i--;
                        while (img.at<uchar>(i, j - 4) == 0)i--;
                        ver[1][0] = i;
                        ver[1][1] = j;
                        endFor = 4; // clockwise rotation
                        break;
                    }
                }
            }
        }
        // bottom left or right detection
        if (endFor == 3) {// anticlockwise rotation
            // right bottom
            for (int j = rightEdge; j > WIDTH / 2 && endFor == 3; j--) {
                for (int i = HEIGHT - 2; i > HEIGHT / 2; i--) {
                    if (img.at<uchar>(i + 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j - 1) == 0)i++;
                        while (img.at<uchar>(i, j - 2) == 0)i++;
                        while (img.at<uchar>(i, j - 3) == 0)i++;
                        while (img.at<uchar>(i, j - 4) == 0)i++;
                        ver[2][0] = i;
                        ver[2][1] = j;
                        endFor = 0;
                        break;
                    }
                }
            }
            // left bottom
            for (int i = HEIGHT - 1; i > HEIGHT / 2; i--) {
                for (int j = leftEdge; j < WIDTH / 2; j++) {
                    if (img.at<uchar>(i, j - 1) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i - 1, j) == 0) { j--; }
                        while (img.at<uchar>(i - 2, j) == 0) { j--; }
                        while (img.at<uchar>(i - 3, j) == 0) { j--; }
                        while (img.at<uchar>(i - 4, j) == 0) { j--; }
                        ver[3][0] = i;
                        ver[3][1] = j;
                        return;
                    }
                }
            }
        }
        if (endFor == 4) {// clockwise rotation
            // right bottom
            for (int i = HEIGHT - 2; i > HEIGHT / 2 && endFor == 4; i--) {
                for (int j = rightEdge; j > WIDTH / 2; j--) {
                    if (img.at<uchar>(i, j + 1) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i - 1, j) == 0)j++;
                        while (img.at<uchar>(i - 2, j) == 0)j++;
                        while (img.at<uchar>(i - 3, j) == 0)j++;
                        while (img.at<uchar>(i - 4, j) == 0)j++;
                        ver[2][0] = i;
                        ver[2][1] = j;
                        endFor = 0;
                        break;
                    }
                }
            }
            // left bottom
            for (int j = leftEdge; j < WIDTH / 2; j++) {
                for (int i = HEIGHT - 2; i > HEIGHT / 2; i--) {
                    if (img.at<uchar>(i + 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j + 1) == 0)i++;
                        while (img.at<uchar>(i, j + 2) == 0)i++;
                        while (img.at<uchar>(i, j + 3) == 0)i++;
                        while (img.at<uchar>(i, j + 4) == 0)i++;
                        ver[3][0] = i;
                        ver[3][1] = j;
                        return;
                    }
                }
            }
        }
    }
}

int allDetection(Mat img, int pos) {
    int lenFlag = 0;
    if (pos == 1) {
        vertexDetection(img, pos); // vertex detection
        calLength(); // calculate the length of all sides
        lenFlag = checkLength();
        if (lenFlag != 0) {
            return lenFLag;
        }
        

        
    }
}
void calLength() {
    topLength = vertex[1][1] - vertex[0][1];
    rightLength = vertex[2][0] - vertex[1][0];
    bottomLength = vertex[2][1] - vertex[3][1];
    leftLength = vertex[3][0] - vertex[0][0];
}
int checkLength() {
    if (leftLength - rightLength > LERROR || leftLength - rightLength < -LERROR) {
        return 1;
    }
    if (topLength - bottomLength > LERROR || topLength - bottomLength < -LERROR) {
        return 2;
    }
    return 0;
}

int main() {
    // Read image
    string root = "../images/";
    string directoryName = "FrontError";
    string fileName = "05.bmp";
    string fileLocation = root + directoryName + '/' + fileName;
    cout << fileLocation << endl;
    src = imread(fileLocation);
    if (src.empty()) {
        return -1;
    }
    // namedWindow("origin", 0); // Show the original image - 0 means the window can be dragged
    // imshow("origin", src);
    /* Preprocess the image begin */
    cvtColor(src, src_gray, COLOR_RGB2GRAY); // convert the img to gray 
    threshold(src_gray, dst, 120, 250.0, THRESH_BINARY); // Binaryzation
    /* Preprocess the image end */

    /* Detect silicon slice begin */
    int pos = posDetection(dst);
    cout << "position: \n" << pos << endl;

    getEdge(dst); // get the start point of detecting
    cout << "left edge: " << leftEdge << "\t" << "right edge: " << rightEdge << endl;

    m = dst.clone();
    for (int i = 0; i < m.rows; i++) { // convert the img to be overall black
        for (int j = 0; j < m.cols; j++) {
            m.at<uchar>(i, j) = 0;
        }
    }

    int allFlag = allDetection(pos, dstimage); // detection


    /* Detect silicon slice end */
    waitKey(0); // prevent the image window from closing too quickly

    namedWindow("gray", 0);
    imshow("gray", dst);
    return 0;
}


