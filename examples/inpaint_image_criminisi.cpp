/**
   This file is part of Inpaint.

   Copyright Christoph Heindl 2014

   Ioobar is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   Inpaint is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with Inpaint.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <inpaint/criminisi_inpainter.h>

#include <iostream>
#include <opencv2/opencv.hpp>

struct ImageInfo {
	cv::Mat image;
	cv::Mat mask;
	cv::Mat displayImage;
	bool mouseDown;
	int patchSize;
};

void onMouse(int eventType, int x, int y, int flags, void* data)
{
	ImageInfo &ii = *reinterpret_cast<ImageInfo*>(data);
	if (eventType == cv::EVENT_LBUTTONDOWN)
		ii.mouseDown = true;
	else if (eventType == cv::EVENT_LBUTTONUP)
		ii.mouseDown = false;

	if (!ii.mouseDown)
        return;

	cv::circle(ii.mask, cv::Point(x, y), ii.displayImage.rows / 40, cv::Scalar(255), -1);
	ii.displayImage.setTo(cv::Scalar(0,250,0), ii.mask);
	cv::imshow("Image Inpaint", ii.displayImage);
}

/** Main entry point */
int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << argv[0] << " image.png" << std::endl;
		return -1;
	}

	cv::Mat inputImage = cv::imread(argv[1]);

	ImageInfo ii;
	ii.mouseDown = false;
	ii.patchSize = 9;

	ii.image = inputImage.clone();
	ii.displayImage = ii.image.clone();
	ii.mask.create(ii.image.size(), CV_8UC1);
	ii.mask.setTo(0);

	cv::namedWindow("Image Inpaint", CV_WINDOW_FREERATIO);
	cv::setMouseCallback("Image Inpaint", onMouse, &ii);
	//cv::createTrackbar("Patchsize", "Image Inpaint", &ii.patchSize, 20);
	
	bool done = false;
	bool editingMode = true;

    Inpaint::CriminisiInpainter inpainter;
	cv::Mat image, mask;	
	while (!done) {
		if (editingMode) {
			image = ii.displayImage.clone();
		} else {
			if (inpainter.hasMoreSteps()) {
				inpainter.step(image, mask);
				image.setTo(cv::Scalar(0,250,0), mask);
			} else {
				ii.image = inpainter.image();
				ii.displayImage = ii.image.clone();
				ii.mask = mask;
				ii.displayImage.setTo(cv::Scalar(0,250,0), ii.mask);
				editingMode = true;
			}
		}

		cv::imshow("Image Inpaint", image);
		int key = cv::waitKey(10);

		if (key == 'x') {
			done = true;
		} else if (key == 'e') {
			if (editingMode) {
				// Was in editing, now perform
				inpainter.setImage(ii.image);
				inpainter.setMask(ii.mask);
				inpainter.setPatchSize(ii.patchSize);
				inpainter.initialize();
			} else {
				// Was performing, allow editing based on current progress
				ii.image = inpainter.image();
				ii.displayImage = ii.image.clone();
				ii.mask = mask;
				ii.displayImage.setTo(cv::Scalar(0,250,0), ii.mask);
			}
			editingMode = !editingMode;
		} else if (key == 'r') {
			// revert
			ii.image = inputImage.clone();
			ii.displayImage = ii.image.clone();
			ii.mask.create(ii.image.size(), CV_8UC1);
			ii.mask.setTo(0);
			editingMode = true;
		}
	}

	cv::imshow("source", inputImage);
	cv::imshow("final", inpainter.image());
	cv::waitKey();
	cv::imwrite("final.png", inpainter.image());

	return 0;
}




