#include <opencv2/opencv.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void mythreshold(Mat &img, uchar T, bool flag)
{
	int n1 = img.rows;
	int nc = img.cols * img.channels();
	if (img.isContinuous())//�ж�ͼ���Ƿ�����
	{
		nc = nc * n1;
		n1 = 1;
	}
	for (int i = 0; i < n1; i++)
	{
		uchar *p = img.ptr<uchar>(i);
		for (int j = 0; j < nc; j++)
		{
			if (flag == true)
			{
				if (p[j] < T)
					p[j] = 0;
				else p[j] = 255;
			}
			if (flag == false)
			{
				if (p[j] > T)
					p[j] = 0;
				else p[j] = 255;
			}
		}
	}
}

void add_logo(Mat &img, Mat &logo, int thresh, Point start)
{
	//��ֵ����������
	Mat logo_gray;
	cvtColor(logo, logo_gray, COLOR_BGR2GRAY);
	mythreshold(logo_gray, thresh, false);
	//imshow("gray", logo_gray);
	Mat mask = logo_gray;

	//��ԭͼ���ںϵ�����
	Rect r1(start.x, start.y, logo.cols, logo.rows);
	Mat img_show = img(r1);
	
	//ͨ���������logo
	logo.copyTo(img_show, mask);
}

void detectAndDraw(Mat& img, CascadeClassifier& cascade,
	double scale, bool tryflip, Point &cen, int &rad)
{
	int i = 0;
	double t = 0;
	//�������ڴ����������������
	vector<Rect> faces, faces2;
	//����һЩ��ɫ��������ʾ��ͬ������
	const static Scalar colors[] = { CV_RGB(0, 0, 255),
		CV_RGB(0, 128, 255),
		CV_RGB(0, 255, 255),
		CV_RGB(0, 255, 0),
		CV_RGB(255, 128, 0),
		CV_RGB(255, 255, 0),
		CV_RGB(255, 0, 0),
		CV_RGB(255, 0, 255) };
	//������С��ͼƬ���ӿ����ٶ�
	//nt cvRound (double value) ��һ��double�͵��������������룬������һ����������
	Mat gray, smallImg(cvRound(img.rows / scale), cvRound(img.cols / scale), CV_8UC1);
	//ת�ɻҶ�ͼ��Harr�������ڻҶ�ͼ
	cvtColor(img, gray, CV_BGR2GRAY);
	//�ı�ͼ���С��ʹ��˫���Բ�ֵ
	resize(gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR);
	//�任���ͼ�����ֱ��ͼ��ֵ������
	equalizeHist(smallImg, smallImg);

	//����ʼ�ͽ�������˺�����ȡʱ�䣬������������㷨ִ��ʱ��
	t = (double)cvGetTickCount();
	//�������
	//detectMultiScale������smallImg��ʾ����Ҫ��������ͼ��ΪsmallImg��faces��ʾ��⵽������Ŀ�����У�1.1��ʾ
	//ÿ��ͼ��ߴ��С�ı���Ϊ1.1��2��ʾÿһ��Ŀ������Ҫ����⵽3�β��������Ŀ��(��Ϊ��Χ�����غͲ�ͬ�Ĵ��ڴ�
	//С�����Լ�⵽����),CV_HAAR_SCALE_IMAGE��ʾ�������ŷ���������⣬��������ͼ��Size(30, 30)ΪĿ���
	//��С���ߴ�
	cascade.detectMultiScale(smallImg, faces,
		1.1, 2, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		| CV_HAAR_SCALE_IMAGE
		,
		Size(30, 30));
	//���ʹ�ܣ���תͼ��������
	if (tryflip)
	{
		/*flipCode����תģʽ��flipCode == 0��ֱ��ת����X�ᷭת����flipCode>0ˮƽ��ת����Y�ᷭת����
			flipCode<0ˮƽ��ֱ��ת������X�ᷭת������Y�ᷭת���ȼ�����ת180�㣩*/
		flip(smallImg, smallImg, 1);
		cascade.detectMultiScale(smallImg, faces2,
			1.1, 2, 0
			//|CV_HAAR_FIND_BIGGEST_OBJECT
			//|CV_HAAR_DO_ROUGH_SEARCH
			| CV_HAAR_SCALE_IMAGE
			,
			Size(30, 30));
		for (vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++)
		{
			faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
		}
	}
	t = (double)cvGetTickCount() - t;
	//   qDebug( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
	for (vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++)
	{
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[i % 8];
		int radius;

		double aspect_ratio = (double)r->width / r->height;
		if (0.75 < aspect_ratio && aspect_ratio < 1.3)
		{
			//��ʾ����ʱ����С֮ǰ��ͼ���ϱ�ʾ����������������ű��������ȥ
			center.x = cvRound((r->x + r->width*0.5)*scale);
			center.y = cvRound((r->y + r->height*0.5)*scale);
			radius = cvRound((r->width + r->height)*0.25*scale);
			circle(img, center, radius, color, 3, 8, 0);

			cen = center;
			rad = radius;

		}
		else
			rectangle(img, cvPoint(cvRound(r->x*scale), cvRound(r->y*scale)),
			cvPoint(cvRound((r->x + r->width - 1)*scale), cvRound((r->y + r->height - 1)*scale)),
			color, 3, 8, 0);
		//if (nestedCascade.empty())
		//	continue;
		//smallImgROI = smallImg(*r);
		////ͬ�������������
		//nestedCascade.detectMultiScale(smallImgROI, nestedObjects,
		//	1.1, 2, 0
		//	//|CV_HAAR_FIND_BIGGEST_OBJECT
		//	//|CV_HAAR_DO_ROUGH_SEARCH
		//	//|CV_HAAR_DO_CANNY_PRUNING
		//	| CV_HAAR_SCALE_IMAGE
		//	,
		//	Size(30, 30));
		//for (vector<Rect>::const_iterator nr = nestedObjects.begin(); nr != nestedObjects.end(); nr++)
		//{
		//	cout << "eyes" << endl;
		//	center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
		//	center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
		//	radius = cvRound((nr->width + nr->height)*0.25*scale);
		//	circle(img, center, radius, color, 3, 8, 0);
		//}
	}
	imshow("result", img);
}

void output_text()
{
	//�����ӭ��Ϣ��OpenCV�汾
	printf("\n\n\t\t\tȤζС��Ŀ֮��ñ��\n");
	printf("\n\n\t\t\tOpenCV�汾Ϊ��" CV_VERSION);
	printf("\n");
	printf("----------------------------------------------------------------------------\n");
	printf("�������裺\n");
	printf("\tSpace������ñ��\n");
	printf("\tEsc����  �˳�����\n");
	printf("----------------------------------------------------------------------------\n");
}

/***************************������ֵ*********************************/
Mat test_hat = imread("E:/mygithub/WearHat/hat1.png", -1);
int test_threshold = 90;
void on_Trackbar(int, void*)
{
	Mat test_img = imread("E:/mygithub/WearHat/img.jpg", 1);
	//��ֵ����������
	Mat logo_gray;
	cvtColor(test_hat, logo_gray, COLOR_BGR2GRAY);
	mythreshold(logo_gray, test_threshold, false);
	imshow("gray", logo_gray);
	Mat mask = logo_gray;
	imwrite("hat1_mask.jpg", logo_gray);

	//��ԭͼ���ںϵ�����
	Rect r1(50, 50, logo_gray.cols, logo_gray.rows);
	Mat img_show = test_img(r1);

	//ͨ���������logo
	test_hat.copyTo(img_show, mask);
	imshow("���logo", test_img);

	cout << "�켣��λ�ã�" << getTrackbarPos("��ֵ", "test") << endl;
}
/***************************������ֵ*********************************/

int main(int argc, char **argv)
{
	output_text();
	/***************************������ֵ*********************************/
	//�켣��
	//resize(test_hat, test_hat, Size(400, 400));
	//imshow("test_hat", test_hat);
	//imwrite("hat_3.png", test_hat);

	//hat1: 90
	//hat2: 62
	//hat3: 246

	//namedWindow("test", 0);
	//createTrackbar("��ֵ", "test", &test_threshold, 255, on_Trackbar);
	//on_Trackbar(test_threshold, 0);
	/***************************������ֵ*********************************/

	//�������
	CascadeClassifier cascade, nestedCascade;
	//ѵ���õ��ļ����ƣ������ڿ�ִ���ļ�ͬĿ¼��
	cascade.load("E:/mygithub/WearHat/haarcascade_frontalface_alt.xml");


	int hat_number = 1;		//ñ������
	Mat img;
	VideoCapture cap(0);
	cout << "ñ������Ϊ: " << 1 << endl;
	while (1)
	{
		cap >> img;
		if (img.empty()) break;
		//imshow("img", img);

		Point center;
		int radius = 0;
		Mat img_dete;
		img.copyTo(img_dete);
		detectAndDraw(img_dete, cascade, 2, 0, center, radius);
		//imshow("img_dete", img_dete);
		//cout << "Բ��:" << center << " ,�뾶:" << radius << endl;

		Mat img_temp;
		if (center.x != 0 && radius != 0)
		{
			Mat hat;//logo����
			int my_thresh;		//��ֵ
			Point add_point = Point(0, 0);	//���logo��ʼ��
			switch (hat_number)
			{
			case 1: hat = imread("E:/mygithub/WearHat/hat1.png", -1); //�����Alphaͼ��
				my_thresh = 90;
				resize(hat, hat, Size(hat.cols * radius / 180, hat.rows * radius / 180));
				add_point = Point(center.x - (hat.cols >> 1), center.y - radius - (50 + 130 * radius / 180)); //��ʼ��
				if (add_point.x < 0 || add_point.y < 0)
					add_point = Point(0, 0);
				break;
			case 2: hat = imread("E:/mygithub/WearHat/hat2.png", -1); //�����Alphaͼ��
				my_thresh = 62;
				resize(hat, hat, Size(hat.cols * radius / 200, hat.rows * radius / 200));
				add_point = Point(center.x - (hat.cols >> 1) - 40, center.y - radius - (40 + 130 * radius / 200)); //��ʼ��
				if (add_point.x < 0 || add_point.y < 0)
					add_point = Point(0, 0);
				break;
			case 3: hat = imread("E:/mygithub/WearHat/hat3.png", -1); //�����Alphaͼ��
				my_thresh = 246;
				resize(hat, hat, Size(hat.cols * radius / 170, hat.rows * radius / 170));
				add_point = Point(center.x - (hat.cols >> 1) + 15, center.y - radius - (130 + 130 * radius / 190)); //��ʼ��
				if (add_point.x < 0 || add_point.y < 0)
					add_point = Point(0, 0);
				break;
			case 4: cout << "�����ڴ�" << endl;
				break;
			}

			if (hat.empty())
				continue;
			//imshow("hat", hat);

			//���logo
			if (add_point.x != 0 && add_point.y != 0)
			{
				img.copyTo(img_temp);
				add_logo(img_temp, hat, my_thresh, add_point);
			}
			else img_temp = img;
		}
		else 
			img_temp = img;

		imshow("���logo", img_temp);

		char key = waitKey(30);
		switch (key)
		{
			case 27: return 0; break;
			case 32: hat_number++;
				if (hat_number == 4)
				{
					hat_number = 1;
					cout << "ñ������Ϊ: " << hat_number << endl;
				}
				else
					cout << "ñ������Ϊ: " << hat_number << endl;

				break;
		}
	}

	//����Դͼ

	//img = imread("E:/mygithub/WearHat/img.jpg", 1); //����Դͼ��
	//if (img.empty()) return -1;
	//imshow("img", img);






	//waitKey(0);
	return 0;
	
}