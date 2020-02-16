#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
using namespace std;
using namespace cv;

// global variables
#define WIDTH 640 // width of the image
#define HEIGHT 480 // height of the image
#define LENGTH 446 // the measurement of the silicon slice
#define DEPTH 5 // defect depth
#define LERROR 15 // the range of length difference

int leftBorder; // the left start point of the img
int rightBorder; // the right start point of the img
int topLength = 0, bottomLength = 0, leftLength = 0, rightLength = 0;
int leftEdge[HEIGHT] = {0}, rightEdge[HEIGHT] = {0}, topEdge[WIDTH] = {0}, bottomEdge[WIDTH] = {0};
float kTop = 0, kRight = 0, kBottom = 0, kLeft = 0; // slope of four lines
float bTop = 0, bRight = 0, bBottom = 0, bLeft = 0; // intercept of four lines
int jEstimate1 = 0, jEstimate2 = 0, jEstimate3 = 0; // estimate j value
int jError1 = 0, jError2 = 0, jError3 = 0; // the difference of estimate value and actual value
int iEstimate1 = 0, iEstimate2 = 0, iEstimate3 = 0; // estimate i value
int iError1 = 0, iError2 = 0, iError3 = 0; // the difference of estimate value and actual value

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

int rightCheck();
int leftCheck();
int topCheck();
int bottomCheck();

void fitLine();
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
            leftBorder = j + 1;
            break;
        }
    }
    for (int j = WIDTH - 2; j > 1; j--) {
        if (img.at<uchar>(HEIGHT / 2, j + 1) == 0 
                && img.at<uchar>(HEIGHT / 2, j) == 250
                && img.at<uchar>(HEIGHT / 2, j - 1) == 250) {
            rightBorder = j - 1;
            break;
        }
    }
    leftBorder += 10;
    rightBorder -= 10;
}

void edgeDetection(Mat img, int pos) {
    if (pos == 1) {
        // the first situation doesn't need top edge detection
        //
        // left edge detection
        int j = vertex[0][1];
        for (int i = vertex[0][0] + 1; i < vertex[3][0] - 4; i++) { // left edge 
            if (img.at<uchar>(i, j) == 0) {
                for (; j > leftBorder; j--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j - 1) == 250) {
                        leftEdge[i] = j;
                        break;
                    }
                }
            } else {  // the current pixel is 250, so it should search towards right
                for (; j < WIDTH / 2; j++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j - 1) == 250) {
                        leftEdge[i] = j;
                        break;
                    }
                }
            }
        }
        // right edge detection
        j = vertex[1][1];
        for (int i = vertex[1][0] + 1; i < vertex[2][0] - 4; i++) {
            if (img.at<uchar>(i, j) == 0) { // the current pixel is 0, it should search towards right
                for (; j < rightBorder; j++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                        rightEdge[i] = j;
                        break;
                    }
                }
            } else {
                for  (; j > WIDTH / 2; j--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                        rightEdge[i] = j;
                        break;
                    }
                }
            }
        }
        // bottom edge detection
        int i = vertex[3][0];
        for (int j = vertex[3][1] + 1; j < vertex[2][1] - 4; j++) {
            if (img.at<uchar>(i, j) == 0) { // the current pixel is 0, it should search towards bottom
                for (; i < HEIGHT / 2; i++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i + 1, j) == 250) {
                        bottomEdge[j] = i;
                        break;
                    }
                }
            } else { // the current pixel is 250, it shuold search towards top
                for (; i > 1; i--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i + 1, j) == 250) {
                        bottomEdge[j] = i;
                        break;
                    }
                }
            }
        }
    } else if (pos == 2) {
        int j = vertex[0][1];
        for (int i = vertex[0][0] + 1; i < vertex[3][0] - 4; i++) { // left edge 
            if (img.at<uchar>(i, j) == 0) {
                for (; j > leftBorder; j--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j - 1) == 250) {
                        leftEdge[i] = j;
                        break;
                    }
                }
            } else {  // the current pixel is 250, so it should search towards right
                for (; j < WIDTH / 2; j++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j - 1) == 250) {
                        leftEdge[i] = j;
                        break;
                    }
                }
            }
        }
        // right edge detection
        j = vertex[1][1];
        for (int i = vertex[1][0] + 1; i < vertex[2][0] - 4; i++) {
            if (img.at<uchar>(i, j) == 0) { // the current pixel is 0, it should search towards right
                for (; j < rightBorder; j++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                        rightEdge[i] = j;
                        break;
                    }
                }
            } else {
                for  (; j > WIDTH / 2; j--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                        rightEdge[i] = j;
                        break;
                    }
                }
            }
        }
        // bottom edge detection
        int i = vertex[3][0];
        for (int j = vertex[3][1] + 1; j < vertex[2][1] - 4; j++) {
            if (img.at<uchar>(i, j) == 0) { // the current pixel is 0, it should search towards bottom
                for (; i < HEIGHT - 5; i++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i + 1, j) == 250) {
                        bottomEdge[j] = i;
                        break;
                    }
                }
            } else { // the current pixel is 250, it shuold search towards top
                for (; i > 1; i--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i + 1, j) == 250) {
                        bottomEdge[j] = i;
                        break;
                    }
                }
            }
        }
    } else if (pos == 3) {
        int j = vertex[0][1];
        for (int i = vertex[0][0] + 1; i < vertex[3][0] - 4; i++) { // left edge 
            if (img.at<uchar>(i, j) == 0) {
                for (; j > leftBorder; j--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j - 1) == 250) {
                        leftEdge[i] = j;
                        break;
                    }
                }
            } else {  // the current pixel is 250, so it should search towards right
                for (; j < WIDTH / 2; j++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j - 1) == 250) {
                        leftEdge[i] = j;
                        break;
                    }
                }
            }
        }
        // right edge detection
        j = vertex[1][1];
        for (int i = vertex[1][0] + 1; i < vertex[2][0] - 4; i++) {
            if (img.at<uchar>(i, j) == 0) { // the current pixel is 0, it should search towards right
                for (; j < rightBorder; j++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                        rightEdge[i] = j;
                        break;
                    }
                }
            } else {
                for  (; j > WIDTH / 2; j--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i, j + 1) == 250) {
                        rightEdge[i] = j;
                        break;
                    }
                }
            }
        }
        // bottom edge detection
        int i = vertex[3][0];
        for (int j = vertex[3][1] + 1; j < vertex[2][1] - 4; j++) {
            if (img.at<uchar>(i, j) == 0) { // the current pixel is 0, it should search towards bottom
                for (; i < HEIGHT - 5; i++) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i + 1, j) == 250) {
                        bottomEdge[j] = i;
                        break;
                    }
                }
            } else { // the current pixel is 250, it shuold search towards top
                for (; i > 1; i--) {
                    if (img.at<uchar>(i, j) == 0 && img.at<uchar>(i + 1, j) == 250) {
                        bottomEdge[j] = i;
                        break;
                    }
                }
            }
        }
        // top edge detection
        i = vertex[0][0];
        for (int j = vertex[0][1] + 1; j < vertex[1][1] - 4; j++) {
            if (img.at<uchar>(i, j) == 250) {
                for (; i < HEIGHT / 2; i++) {
                    if (img.at<uchar>(i, j) == 250 && img.at<uchar>(i + 1, j) == 0) {
                        topEdge[j] = i + 1;
                        break;
                    }
                }
            } else {
                for (; i > 4; i--) {
                    if (img.at<uchar>(i, j) == 250 && img.at<uchar>(i + 1, j) == 0) {
                        topEdge[j] = i + 1;
                        break;
                    }
                }
            }
        }
    }
}
void vertexDetection(Mat img, int pos) {
    if (pos == 1) {
        // check the left top vertex
        for (int j = leftBorder; j < rightBorder; j++) {
            if (img.at<uchar>(0, j - 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[0][0] = 0;
                vertex[0][1] = j;
                break;
            }
        }
        // check the right top vertex
        for (int j = rightBorder - 1; j > leftBorder; j--) {
            if (img.at<uchar>(0, j + 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[1][0] = 0;
                vertex[1][1] = j;
                break;
            }
        }
        // check the bottom vertex
        int endFor = 0;
        for (int i = HEIGHT / 2; i > 4 && endFor == 0; i--) {
            for (int j = rightBorder; j > leftBorder; j--) {
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
                        vertex[3][0] = j;
                        for (int k = rightBorder; k > WIDTH / 2; k--) {
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
            for (int j = leftBorder; j < WIDTH / 2; j++) {
                for (int i = HEIGHT / 2; i > 1; i--) {
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
    } else if (pos == 2) {
         // check the left top vertex
        for (int j = leftBorder; j < rightBorder; j++) {
            if (img.at<uchar>(0, j - 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[0][0] = 0;
                vertex[0][1] = j;
                break;
            }
        }
        // check the right top vertex
        for (int j = rightBorder - 1; j > leftBorder; j--) {
            if (img.at<uchar>(0, j + 1) == 250 && img.at<uchar>(0, j) == 0) {
                vertex[1][0] = 0;
                vertex[1][1] = j;
                break;
            }
        }
        // check the bottom vertex
        int endFor = 0;
        for (int i = HEIGHT - 5; i > HEIGHT / 2 && endFor == 0; i--) {

            for (int j = rightBorder; j > leftBorder; j--) {
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
                        for (int k = rightBorder; k > WIDTH / 2; k--) {
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
            for (int j = leftBorder; j < WIDTH  / 2; j++) {
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
        for (int i = 1; i < HEIGHT / 2 && endFor == 0; i++) {
            for (int j = leftBorder; j < rightBorder; j++) {
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
                        vertex[0][0] = i;
                        vertex[0][1] = j;
                        endFor = 2;
                        break;
                    }
                }
            }
        }
        if (endFor == 1) {// lost the left top vertex
            for (int j = leftBorder; j < WIDTH / 2 && endFor == 1; j++) {
                for (int i = 1; i < HEIGHT / 2; i++) {
                    if (img.at<uchar>(i - 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j + 1) == 0) { i--; }
                        while (img.at<uchar>(i, j + 2) == 0) { i--; }
                        while (img.at<uchar>(i, j + 3) == 0) { i--; }
                        while (img.at<uchar>(i, j + 4) == 0) { i--; }
                        vertex[0][0] = i;
                        vertex[0][1] = j;
                        endFor = 3; // anticlockwise rotation
                        break;
                    }
                }
            }
        }
        if (endFor == 2) {// lost the right top vertex
            for (int j = rightBorder; j > WIDTH / 2 && endFor == 2; j--) {
                for (int i = 1; i < HEIGHT / 2; i++) {
                    if (img.at<uchar>(i - 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j - 1) == 0)i--;
                        while (img.at<uchar>(i, j - 2) == 0)i--;
                        while (img.at<uchar>(i, j - 3) == 0)i--;
                        while (img.at<uchar>(i, j - 4) == 0)i--;
                        vertex[1][0] = i;
                        vertex[1][1] = j;
                        endFor = 4; // clockwise rotation
                        break;
                    }
                }
            }
        }
        // bottom left or right detection
        if (endFor == 3) {// anticlockwise rotation
            // right bottom
            for (int j = rightBorder; j > WIDTH / 2 && endFor == 3; j--) {
                for (int i = HEIGHT - 2; i > HEIGHT / 2; i--) {
                    if (img.at<uchar>(i + 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j - 1) == 0)i++;
                        while (img.at<uchar>(i, j - 2) == 0)i++;
                        while (img.at<uchar>(i, j - 3) == 0)i++;
                        while (img.at<uchar>(i, j - 4) == 0)i++;
                        vertex[2][0] = i;
                        vertex[2][1] = j;
                        endFor = 0;
                        break;
                    }
                }
            }
            // left bottom
            for (int i = HEIGHT - 1; i > HEIGHT / 2; i--) {
                for (int j = leftBorder; j < WIDTH / 2; j++) {
                    if (img.at<uchar>(i, j - 1) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i - 1, j) == 0) { j--; }
                        while (img.at<uchar>(i - 2, j) == 0) { j--; }
                        while (img.at<uchar>(i - 3, j) == 0) { j--; }
                        while (img.at<uchar>(i - 4, j) == 0) { j--; }
                        vertex[3][0] = i;
                        vertex[3][1] = j;
                        return;
                    }
                }
            }
        }
        if (endFor == 4) {// clockwise rotation
            // right bottom
            for (int i = HEIGHT - 2; i > HEIGHT / 2 && endFor == 4; i--) {
                for (int j = rightBorder; j > WIDTH / 2; j--) {
                    if (img.at<uchar>(i, j + 1) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i - 1, j) == 0)j++;
                        while (img.at<uchar>(i - 2, j) == 0)j++;
                        while (img.at<uchar>(i - 3, j) == 0)j++;
                        while (img.at<uchar>(i - 4, j) == 0)j++;
                        vertex[2][0] = i;
                        vertex[2][1] = j;
                        endFor = 0;
                        break;
                    }
                }
            }
            // left bottom
            for (int j = leftBorder; j < WIDTH / 2; j++) {
                for (int i = HEIGHT - 2; i > HEIGHT / 2; i--) {
                    if (img.at<uchar>(i + 1, j) == 250 && img.at<uchar>(i, j) == 0) {
                        while (img.at<uchar>(i, j + 1) == 0)i++;
                        while (img.at<uchar>(i, j + 2) == 0)i++;
                        while (img.at<uchar>(i, j + 3) == 0)i++;
                        while (img.at<uchar>(i, j + 4) == 0)i++;
                        vertex[3][0] = i;
                        vertex[3][1] = j;
                        return;
                    }
                }
            }
        }
    }
}
int allDetection(Mat img, int pos) {
    int lenFlag = 0;
    if (pos == 1 || pos == 2) {
        vertexDetection(img, pos); // vertex detection
        calLength(); // calculate the length of all sides
        lenFlag = checkLength();

        edgeDetection(img, pos); // edge extraction / detection
        fitLine(); // fit the line
        
        if (lenFlag != 0) {
            return lenFlag;
        }
        int rightFlag = 0, bottomFlag = 0, leftFlag = 0;
        // check right edge
        rightFlag = rightCheck();
        if (rightFlag == 1) {
            return 3; // right edge isn't qualified
        }
        // check bottom edge
        bottomFlag = bottomCheck();
        if (bottomFlag == 1) {
            return 4; // bottom edge isn't qualified
        }
        // check left edge
        leftFlag = leftCheck();
        if (leftFlag == 1) {
            return 5; // left edge isn't qualified
        }
    } else if (pos == 3){
        vertexDetection(img, pos);
        calLength();
        lenFlag = checkLength();

        edgeDetection(img, pos);
        fitLine();
        if (lenFlag != 0) {
            return lenFlag;
        }
        int rightFlag = 0, bottomFlag = 0, leftFlag = 0, topFlag = 0;
        // check top edge
        topFlag = topCheck();
        if (topFlag == 1) {
            return 6;
        }
        // check right edge
        rightFlag = rightCheck();
        if (rightFlag == 1) {
            return 3; // right edge isn't qualified
        }
        // check bottom edge
        bottomFlag = bottomCheck();
        if (bottomFlag == 1) {
            return 4; // bottom edge isn't qualified
        }
        // check left edge
        leftFlag = leftCheck();
        if (leftFlag == 1) {
            return 5; // left edge isn't qualified
        }
    } else if (pos == 4) {
        
    } else if (pos == 5) {
        
    } else {
        
    }
    return 0;
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


void printVertex() {
    cout << "v1: (" << vertex[0][0] << ", " << vertex[0][1] << ")" << endl;
    cout << "v2: (" << vertex[1][0] << ", " << vertex[1][1] << ")" << endl;
    cout << "v3: (" << vertex[2][0] << ", " << vertex[2][1] << ")" << endl;
    cout << "v4: (" << vertex[3][0] << ", " << vertex[3][1] << ")" << endl;
}
void showImg(Mat img, string windowName) {
    namedWindow(windowName, 0); // Show the original image - 0 means the window can be dragged
    imshow(windowName, img);
}
void fitLine() {
    kTop = (vertex[1][0] - vertex[0][0]) * 1.0 / (vertex[1][1] - vertex[0][1]);
    kBottom = (vertex[2][0] - vertex[3][0]) * 1.0 / (vertex[2][1] - vertex[3][1]);
    bTop = vertex[0][0] - kTop * vertex[0][1];
    bBottom = vertex[3][0] - kBottom * vertex[2][1];
    kRight = (vertex[2][1] - vertex[1][1])*1.0 / (vertex[2][0] - vertex[1][0]);//右边斜率,交换建系
    kLeft = (vertex[3][1] - vertex[0][1])*1.0 / (vertex[3][0] - vertex[0][0]);//左边斜率，交换建系
    bRight = vertex[1][1] - kRight * vertex[1][0];
    bLeft = vertex[0][1] - kLeft * vertex[0][0];
}
int rightCheck() {
    for (int i = vertex[1][0] + 3; i <= vertex[2][0] - 3; i++) {
        jEstimate1 = kRight * i + bRight;
        jEstimate2 = kRight * (i + 1) + bRight;
        jEstimate3 = kRight * (i + 2) + bRight;
        jError1 = jEstimate1 - rightEdge[i];
        jError2 = jEstimate2 - rightEdge[i + 1];
        jError3 = jEstimate3 - rightEdge[i + 2];
        m.at<uchar>(i, jEstimate1) = 250;
        if (jError1 > DEPTH) {
            if (jError2 > DEPTH) {
                if (jError3 <= DEPTH && jError3 >= -DEPTH) {
                    return 1;
                }
            }
        }
        if (jError1 < -DEPTH) {
            if (jError3 < -DEPTH) {
                if (jError3 <= DEPTH && jError3 >= -DEPTH) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
int leftCheck() {
    for (int i = vertex[0][0] + 3; i <= vertex[3][0] - 3; i++) {
        jEstimate1 = kLeft * i + bLeft;
        jEstimate2 = kLeft * (i + 1) + bLeft;
        jEstimate3 = kLeft * (i + 2) + bLeft;
        jError1 = jEstimate1 - leftEdge[i];
        jError2 = jEstimate2 - leftEdge[i + 1];
        jError3 = jEstimate3 - leftEdge[i + 2];
        m.at<uchar>(i, jEstimate1) = 250;
        if (jError1 > DEPTH) {//
            if (jError2 > DEPTH) {
                if (jError3 <= DEPTH && jError3 >= -DEPTH) {
                    return 1;
                }
            }
        }
        if (jError1 < -DEPTH) {//
            if (jError2 < -DEPTH) {
                if (jError3 <= DEPTH && jError3 >= -DEPTH) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
int topCheck() {
    for (int j = vertex[0][1] + 3; j <= vertex[1][1] - 3; j++) {
        iEstimate1 = kTop * j + bTop;
        iEstimate2 = kTop * (j + 1) + bTop;
        iEstimate3 = kTop * (j + 2) + bTop;
        iError1 = iEstimate1 - topEdge[j];
        iError2 = iEstimate2 - topEdge[j + 1];
        iError3 = iEstimate3 - topEdge[j + 2];
        m.at<uchar>(iEstimate1, j) = 250;
        if (iError1 > DEPTH) {
            if (iError2 > DEPTH) {
                if (iError3 <= DEPTH - 2 && iError3 >= -DEPTH + 2) {
                    return 1;
                }
            }
        }
        if (iError1 < -DEPTH) {
            if (iError2 < -DEPTH) {
                if (iError3 <= DEPTH && iError3 >= -DEPTH) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
int bottomCheck() {
    for (int j = vertex[0][1] + 3; j <= vertex[1][1] - 3; j++) {
        iEstimate1 = kBottom * j + bBottom;
        iEstimate2 = kBottom * (j + 1) + bBottom;
        iEstimate3 = kBottom * (j + 2) + bBottom;
        iError1 = iEstimate1 - bottomEdge[j];
        iError2 = iEstimate2 - bottomEdge[j + 1];
        iError3 = iEstimate3 - bottomEdge[j + 2];
        m.at<uchar>(iEstimate1, j) = 250;
        if (iError1 > DEPTH) {
            if (iError2 > DEPTH) {
                if (iError3 <= DEPTH && iError3 >= -DEPTH) {
                    return 1;
                }
            }
        }
        if (iError1 < -DEPTH) {
            if (iError2 < -DEPTH) {
                if (iError3 <= DEPTH && iError3 >= -DEPTH) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
void printResult(int flag) {
    cout << "Detection Result: ";
    switch(flag) {
        case 0:
            cout << "The silicon slice is qualified." << endl;
            break;
        case 1:
            cout << "left and right length doesn't match" << endl;
            break;
        case 2:
            cout << "top and bottom length doesn't match" << endl;
            break;
        case 3:
            cout << "right edge isn't qualified" << endl;
            break;
        case 4:
            cout << "bottom edge isn't qualified" << endl;
            break;
        case 5:
            cout << "left edge isn't qualified" << endl;
            break;
        case 6:
            cout << "top edge isn't qualified" << endl;
            break;
    }
}
int main() {
    // Read image
    string root = "../images/";
    string directoryName = "NoError";
    string fileName = "06.bmp";
    string fileLocation = root + directoryName + '/' + fileName;
    cout << fileLocation << endl;
    src = imread(fileLocation);
    if (src.empty()) {
        return -1;
    }

    /* Preprocess the image begin */
    cvtColor(src, src_gray, COLOR_RGB2GRAY); // convert the img to gray 
    threshold(src_gray, dst, 120, 250.0, THRESH_BINARY); // Binaryzation
    /* Preprocess the image end */

    /* Detect silicon slice begin */
    int pos = posDetection(dst);
    cout << "position: " << pos << endl;

    getEdge(dst); // get the start point of detecting
    cout << "left border: " << leftBorder << "\t" << "right border: " << rightBorder << endl;

    m = dst.clone();
    for (int i = 0; i < m.rows; i++) { // convert the img to be overall black
        for (int j = 0; j < m.cols; j++) {
            m.at<uchar>(i, j) = 0;
        }
    }
    int allFlag = allDetection(dst, pos); // detection
    printResult(allFlag);
    printVertex();
    /* draw the edge begin */
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            dst.at<uchar>(i, j) = 0;
            dst.at<uchar>(i, leftEdge[i]) = 250;
            dst.at<uchar>(i, rightEdge[i]) = 250;
            dst.at<uchar>(bottomEdge[j], j) = 250;
            dst.at<uchar>(topEdge[j], j) = 250;
        }
    }
    /* draw the edge end */
    

    for (int i = 0; i < WIDTH; i++) {
        dst.at<uchar>(HEIGHT / 2, i) = 0;
    }
    for (int i = 0; i < dst.rows; i++) {
        for (int j = 0; j < dst.cols; j++) {
            if (m.at<uchar>(i, j) == 250) {
                dst.at<uchar>(i, j) = 250;
            }
        }
    }
    showImg(dst, "gray");
    showImg(m, "fitline");
    /* Detect silicon slice end */
    waitKey(0); // prevent the image window from closing too quickly

    return 0;
}


