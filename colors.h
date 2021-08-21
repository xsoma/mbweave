#pragma once
#include <algorithm>

class color_t {
public:
	color_t()
	{
		*(uint32_t*)_color = 0;
	}

	color_t(int color32)
	{
		*(uint32_t*)_color = color32;
	}

	color_t(int _r, int _g, int _b)
	{
		set(_r, _g, _b, 255);
	}

	color_t(int _r, int _g, int _b, int _a)
	{
		set(_r, _g, _b, _a);
	}

	void set(int _r, int _g, int _b, int _a = 255)
	{
		_color[0] = std::clamp(_r, 0, 255);
		_color[1] = std::clamp(_g, 0, 255);
		_color[2] = std::clamp(_b, 0, 255);
		_color[3] = std::clamp(_a, 0, 255);
	}

	void get(int& _r, int& _g, int& _b, int& _a) const
	{
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	inline int get_red() const
	{
		return _color[0];
	}
	inline int get_green() const
	{
		return _color[1];
	}
	inline int get_blue() const
	{
		return _color[2];
	}
	inline int get_alpha() const
	{
		return _color[3];
	}

	inline void set_red(int value) {
		_color[0] = value;
	}
	inline void set_green(int value) {
		_color[1] = value;
	}
	inline void set_blue(int value) {
		_color[2] = value;
	}
	inline void set_alpha(int value) {
		_color[3] = value;
	}
	uint8_t& operator[](int index)
	{
		return _color[index];
	}

	const uint8_t& operator[](int index) const
	{
		return _color[index];
	}

	const bool& operator ==(const color_t& rhs) const
	{
		return rhs.get_red() == this->get_red()
			&& rhs.get_green() == this->get_green()
			&& rhs.get_blue() == this->get_blue()
			&& rhs.get_alpha() == this->get_alpha();
	}
	


	//new
	uint8_t& r() { return _color[0]; }
	uint8_t& g() { return _color[1]; }
	uint8_t& b() { return _color[2]; }
	uint8_t& a() { return _color[3]; }
	

	static color_t from_hsb(float hue, float saturation, float brightness) {
		float h = hue == 1.0f ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1) {
			return color_t(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 2) {
			return color_t(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 3) {
			return color_t(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
			);
		}
		else if (h < 4) {
			return color_t(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else if (h < 5) {
			return color_t(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else {
			return color_t(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
			);
		}
	}

	static color_t blend(color_t first, color_t second, float t) {
		return color_t(
			first.r() + static_cast<int>(t * (second.r() - first.r())),
			first.g() + static_cast<int>(t * (second.g() - first.g())),
			first.b() + static_cast<int>(t * (second.b() - first.b())),
			first.a() + static_cast<int>(t * (second.a() - first.a()))
		);
	}



	float hue() {
#define RR _color[0]
#define GG _color[1]
#define BB _color[2]
		typedef struct {
			float h, s, v;
		} hsv;
		hsv         out;
		float       min, max, delta;

		min = static_cast<float>(RR < GG ? RR : GG);
		min = static_cast<float>(min < BB ? min : BB);

		max = static_cast<float>(RR > GG ? RR : GG);
		max = static_cast<float>(max > BB ? max : BB);

		out.v = max;
		delta = max - min;
		if (delta < 0.0010f) {
			out.s = 0.f;
			out.h = 0.f;
			return out.h;
		}
		if (max > 0.0f) {
			out.s = (delta / max);
		}
		else {
			out.s = 0.0f;
			out.h = (float)NAN;
			return out.h;
		}
		if (RR >= max)
			out.h = static_cast<float>(GG - BB) / delta;
		else
			if (GG >= max)
				out.h = 2.0f + static_cast<float>(BB - RR) / delta;
			else
				out.h = 4.0f + static_cast<float>(RR - GG) / delta;

		out.h *= 60.0f;
		out.h /= 360.f;

		if (out.h < 0.0f)
			out.h += 360.0f;

		return out.h;
	}

	float brightness() {
#define RR _color[0]
#define GG _color[1]
#define BB _color[2]
		typedef struct {
			float h, s, v;
		} hsv;
		hsv out;

		float min = static_cast<float>(RR < GG ? RR : GG);
		min = static_cast<float>(min < BB ? min : BB);

		float max = static_cast<float>(RR > GG ? RR : GG);
		max = static_cast<float>(max > BB ? max : BB);

		out.v = max;
		float delta = max - min;
		if (delta < 0.0010f) {
			out.s = 0.f;
			out.h = 0.f;
			return out.h;
		}
		if (max > 0.0f) {
			out.s = delta / max;
		}
		else {
			out.s = 0.0f;
			out.h = NAN;
			return out.h;
		}
		if (RR >= max)
			out.h = static_cast<float>(GG - BB) / delta;
		else if (GG >= max)
			out.h = 2.0f + static_cast<float>(BB - RR) / delta;
		else
			out.h = 4.0f + static_cast<float>(RR - GG) / delta;

		out.h *= 60.0f;
		out.h /= 360.f;

		if (out.h < 0.0f)
			out.h += 360.0f;

		return out.v;
	}

	float saturation() {
#define RR _color[0]
#define GG _color[1]
#define BB _color[2]
		typedef struct {
			float h, s, v;
		} hsv;
		hsv out;

		float min = static_cast<float>(RR < GG ? RR : GG);
		min = static_cast<float>(min < BB ? min : BB);

		float max = static_cast<float>(RR > GG ? RR : GG);
		max = static_cast<float>(max > BB ? max : BB);

		out.v = max;
		float delta = max - min;
		if (delta < 0.0010f) {
			out.s = 0.f;
			out.h = 0.f;
			return out.h;
		}
		if (max > 0.0f) {
			out.s = delta / max;
		}
		else {
			out.s = 0.0f;
			out.h = NAN;
			return out.h;
		}
		if (RR >= max)
			out.h = static_cast<float>(GG - BB) / delta;
		else if (GG >= max)
			out.h = 2.0f + static_cast<float>(BB - RR) / delta;
		else
			out.h = 4.0f + static_cast<float>(RR - GG) / delta;

		out.h *= 60.0f;
		out.h /= 360.f;

		if (out.h < 0.0f)
			out.h += 360.0f;

		return out.s;
	}



	
	//









	const bool& operator !=(const color_t& rhs) const
	{
		return !(operator==(rhs));
	}

	color_t& operator=(const color_t& rhs)
	{
		set_u32(rhs.u32());
		return *this;
	}

	void set_u32(uint32_t color32)
	{
		*(uint32_t*)_color = color32;
	}
	inline uint32_t u32() const
	{
		return *(uint32_t*)_color;
	}
	double get_hue()
	{
		double r = _color[0] / 255.f;
		double g = _color[1] / 255.f;
		double b = _color[2] / 255.f;

		double mx = max(r, max(g, b));
		double mn = min(r, min(g, b));
		if (mx == mn)
			return 0.f;

		double delta = mx - mn;

		double hue = 0.f;
		if (mx == r)
			hue = (g - b) / delta;
		else if (mx == g)
			hue = 2.f + (b - r) / delta;
		else
			hue = 4.f + (r - g) / delta;

		hue *= 60.f;
		if (hue < 0.f)
			hue += 360.f;

		return hue / 360.f;
	}

	double get_saturation()
	{
		double r = _color[0] / 255.f;
		double g = _color[1] / 255.f;
		double b = _color[2] / 255.f;

		double mx = max(r, max(g, b));
		double mn = min(r, min(g, b));

		double delta = mx - mn;

		if (mx == 0.f)
			return delta;

		return delta / mx;
	}

	double get_brightness()
	{
		double r = _color[0] / 255.f;
		double g = _color[1] / 255.f;
		double b = _color[2] / 255.f;

		return max(r, max(g, b));
	}

	

	color_t operator+(const color_t& v) const {
		return color_t(get_red() + v.get_red(), get_green() + v.get_green(), get_blue() + v.get_blue(), get_alpha() + v.get_alpha());
	}

	color_t operator*(float f) {
		return color_t(uint8_t(get_red() * f), uint8_t(get_green() * f), uint8_t(get_blue() * f), get_alpha());
	}

	//explicit operator bool() const noexcept {
	//	return (R > 0 || G > 0 || B > 0 || A > 0);
	//}

	static color_t hsb(float hue, float saturation, float brightness)
	{
		auto clamp = [](float value, float min, float max) {
			if (value > max) return max;
			else if (value < min) return min;
			return value;
		};
		hue = std::clamp(hue, 0.f, 1.f);
		saturation = std::clamp(saturation, 0.f, 1.f);
		brightness = std::clamp(brightness, 0.f, 1.f);

		float h = (hue == 1.f) ? 0.f : (hue * 6.f);
		float f = h - static_cast<int>(h);
		float p = brightness * (1.f - saturation);
		float q = brightness * (1.f - saturation * f);
		float t = brightness * (1.f - (saturation * (1.f - f)));

		if (h < 1.f)
			return color_t((int)(brightness * 255), (int)(t * 255), (int)(p * 255));
		else if (h < 2.f)
			return color_t((int)(q * 255), (int)(brightness * 255), (int)(p * 255));
		else if (h < 3.f)
			return color_t((int)(p * 255), (int)(brightness * 255), (int)(t * 255));
		else if (h < 4)
			return color_t((int)(p * 255), (int)(q * 255), (int)(brightness * 255));
		else if (h < 5)
			return color_t((int)(t * 255), (int)(p * 255), (int)(brightness * 255));
		else
			return color_t((int)(brightness * 255), (int)(p * 255), (int)(q * 255));
	}

	color_t transition(const color_t& other, float strength) const {

		static const auto& f = [](const int& a, const int& b, const float& percent) -> const int {
			return a * (1.f - percent) + b * percent;
		};

		if (*this == other)
			return *this;

		return color_t(
			f((*this)[0], other[0], strength),
			f((*this)[1], other[1], strength),
			f((*this)[2], other[2], strength)
		);
	}
	color_t manage_alpha(int alpha) const {
		return color_t((*this)[0], (*this)[1], (*this)[2], std::clamp(alpha, 0, 255));
	}
	color_t increase(int value, bool consider_alpha = false) {
		return color_t((*this)[0] + value, (*this)[1] + value, (*this)[2] + value, (*this)[3] + consider_alpha * value);
	}
private:
	uint8_t _color[4];
};
