// C++ includes
#include <stdio.h>
#include <iostream>
#include <cmath>
// OpenCV includes
#include "opencv4/opencv2/opencv.hpp"
// Because we are lazy...
using namespace std;
using namespace cv;

Mat src;
Mat map_x, map_y;

inline float DegToRad(float x)
{
    return x / 180.0 * M_PI;
}

double d(double i, double j)
{
    return i - j / 2;
}

double r(double x, double y, double w, double h)
{
    return sqrt(d(x, w)*d(x, w) + d(y, h)*d(y, h));
}

double rho(double x, double y, double w, double h, double z)
{
    return (r(x, y, w, h) /z*1.0);
}

double theta(double x, double y, double w, double h, double z)
{
    return 2 * atan(rho(x, y, w, h, z));
}

double a(double x, double y, double w, double h)
{
    return atan2(d(y, h), d(x, w));
}

void polarToCartesian(double  wx, double  wy, double &x, double &y, int width, int height)
{
    double cen_x = width/2.0;
    double cen_y = height/2.0;
    double phi   = 0.0, phi2 = 0;
    double m     = 0;
    double x1    = 0;
    double y1    = 0;
    double x2    = width;
    double y2    = height;
    double xdiff = x2 - x1;
    double ydiff = y2 - y1;
    double xm    = xdiff / 2.0;
    double ym    = ydiff / 2.0;

    double xx, yy;
    double xmax, ymax, rmax;
    double x_calc, y_calc;
    double t;
    double r;

    phi = (2 * M_PI) * (x2 - wx) / xdiff;
    phi = fmod(phi, 2 * M_PI);

    if (phi >= 1.5 * M_PI)
        phi2 = 2 * M_PI - phi;
    else if (phi >= M_PI)
        phi2 = phi - M_PI;
    else if (phi >= 0.5 * M_PI)
        phi2 = M_PI - phi;
    else
        phi2 = phi;

    xx = tan(phi2);
    if (xx != 0)
        m = (double) 1.0 / xx;
    else
        m = 0;

    if (m <= ((double)(ydiff) / (double)(xdiff)))
    {
        if (phi2 == 0)
        {
            xmax = 0;
            ymax = ym - y1;
        }
        else
        {
            xmax = xm - x1;
            ymax = m * xmax;
        }
    }
    else
    {
        ymax = ym - y1;
        xmax = ymax / m;
    }
    
    rmax = sqrt((double)(xmax*xmax + ymax*ymax));
    t    = ((ym - y1) < (xm - x1)) ? (ym - y1) : (xm - x1);
    rmax = (rmax - t) / 100.0 * (100 - 100) + t;        
    r = rmax * (double)((y2 - wy) / (double)(ydiff));
    xx = r * sin(phi2);
    yy = r * cos(phi2);

    if (phi >= 1.5 * M_PI)
    {
        x_calc = (double)xm - xx;
        y_calc = (double)ym - yy;
    }
    else if (phi >= M_PI)
    {
        x_calc = (double)xm - xx;
        y_calc = (double)ym + yy;
    }
    else if (phi >= 0.5 * M_PI)
    {
        x_calc = (double)xm + xx;
        y_calc = (double)ym + yy;
    }
    else
    {
        x_calc = (double)xm + xx;
        y_calc = (double)ym - yy;
    }

    if (x_calc < src.cols && x_calc > 0 && y_calc < src.rows && y_calc > 0)
    {
        x =  x_calc;
        y =  y2 - y_calc;
    }
    else
    {
        x = xm;
        y = ym;
    }
}

int main(int argc, char *argv[])
{
    src              = cv::imread("/tmp/img.png");
    cv::Mat panorama = cv::imread("/tmp/pano.jpg");
    double expand    = src.rows * 10;
    Mat src2         = Mat::zeros(src.rows, expand, CV_8UC3);
    src.copyTo(src2.rowRange(0, src.rows).colRange(expand / 2 - src.rows / 2, expand / 2 + src.rows / 2));

    src              = src2;
    Mat dst          = Mat::zeros(src.rows, src.cols, CV_8UC3);
    map_x.create(src.size(), CV_32FC1);
    map_y.create(src.size(), CV_32FC1);

    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            double x = 0, y = 0;
            double xx = j;
            double yy = i;
            polarToCartesian(j, i, x, y, expand, src.rows);
            
            map_x.at<float>(i, j) = x;
            map_y.at<float>(i, j) = y;
        }
    }

    remap(src, dst, map_x, map_y, INTER_CUBIC, BORDER_WRAP);
    resize(dst, dst, Size(6000, 600), 0.0, 0.0, INTER_LANCZOS4);
    cv::imwrite("/tmp/pastille.jpg", dst);
    dst.copyTo(panorama.rowRange(panorama.rows - 600, panorama.rows).colRange(0, panorama.cols));
    cv::imwrite("/tmp/out.jpg", panorama);

    // generate little planet

    double w        = panorama.cols;
    double h        = panorama.rows;
    double CAMDIST  = 6;
    double WORLDROTATION = 135;
    double z        = w / CAMDIST;
    double rads     = 2.0 * M_PI / (w*1.0);

    WORLDROTATION = DegToRad(WORLDROTATION);

    Mat pixX = Mat::zeros(h, w, CV_32F);
    Mat pixY = Mat::zeros(h, w, CV_32F);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {   
            pixX.at<float>(i, j)=j+1;
        }
    }

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            pixY.at<float>(i, j) = i + 1;
        }
    }

    Mat lat = Mat::zeros(h, w, CV_32F);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            lat.at<float>(i, j) = theta(pixX.at<float>(i, j), pixY.at<float>(i, j),w,h,z);
        }
    }

    Mat lon = Mat::zeros(h, w, CV_32F);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            lon.at<float>(i, j) = a(pixX.at<float>(i, j), pixY.at<float>(i, j), w, h) - M_PI/4.0;
        }
    }

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            lat.at<float>(i, j) = fmod(lat.at<float>(i, j) + M_PI, M_PI) - M_PI / 2.0;
        }
    }

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {   
            lon.at<float>(i, j) = fmod(lon.at<float>(i, j) + M_PI + WORLDROTATION, M_PI*2) - M_PI;
        }

    }

    Mat xe = Mat::zeros(h, w, CV_32F);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            xe.at<float>(i, j) = w / 2.0 - (-lon.at<float>(i, j) / rads);
        }

    }

    Mat ye = Mat::zeros(h, w, CV_32F);

    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            ye.at<float>(i, j) = h / 2.0 - (lat.at<float>(i, j) / rads);
        }

    }

    Mat output;

    remap(panorama, output, xe, ye, INTER_CUBIC,BORDER_WRAP);

    cv::imwrite("/tmp/littlePlanet.jpg", output);

    return 0;
}
