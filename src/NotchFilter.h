#ifndef NOTCH_FILTER_H
#define NOTCH_FILTER_H



class NotchFilter {
public:
	NotchFilter(double frequency, double r);

	double tick(double x0);

private:
	double frequency;
	double r;

	double x1{0}, x2{0};
	double y1{0}, y2{0};

	double w0;

	double a1, a2;
	double b0{1.0f}, b1, b2{1.0f};
};



#endif //NOTCH_FILTER_H
