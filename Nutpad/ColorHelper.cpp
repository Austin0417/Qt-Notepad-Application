#include "ColorHelper.h"

QColor GetRandomColor()
{
	static std::random_device dev;
	static std::mt19937 rng(dev());
	static std::uniform_int_distribution<std::mt19937::result_type> dist(50, 255);

	return QColor(dist(rng), dist(rng), dist(rng));
}