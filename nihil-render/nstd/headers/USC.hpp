#pragma once
#include <iostream>
#include "ScreenRatio.hpp"
#include "WidthHeightEnum.hpp"

namespace nihil {
	namespace nstd {
		class USC
		{
		private:
			static inline int64_t countDigit(int64_t n)
			{
				if (n == 0)
					return 1;
				int count = 0;
				while (n != 0) {
					n = n / 10;
					++count;
				}
				return count;
			}
			static inline int64_t pow(int64_t x, int64_t y)
			{
				int64_t res = 1;
				for (int i = 0; i < y; i++) {
					res = res * x;
				}
				return res;
			}
		public:
			/*
			* @brief converts USC to NDC
			*
			* @param USC - the value to be converted in USC
			* @param ratio - the screen ratio
			* @param xy - if its the x - Width or y - Height (in means of coordinates)
			*/
			static inline float NDC_u(float USC, ScreenRatio ratio, WidthHeightEnum xyz)
			{
				uint8_t denominator;
				if (ratio.width > ratio.height) {
					denominator = ratio.width;
				}
				else {
					denominator = ratio.height;
				}
				if (xyz == WidthHeightEnum::Width || xyz == WidthHeightEnum::Depth) {
					float im1 = USC / 100;
					float im2 = (float)ratio.width;
					float res = ((USC / 100) / ((float)ratio.width)) / 10;

					if (std::isinf(res)) {
						std::cerr << "im1: " << im1 << " im2: " << im2 << std::endl;
						return im1 / im2 / 10;
					}
					else {
						return res;
					}
					return 0.0f;
				}
				if (xyz == WidthHeightEnum::Height) {
					float im1 = USC / 100;
					float im2 = (float)ratio.height;
					float res = ((USC / 100) / ((float)ratio.height)) / 10;

					if (std::isinf(res)) {
						std::cerr << "im1: " << im1 << " im2: " << im2 << std::endl;
						return im1 / im2 / 10;
					}
					else {
						return res;
					}
					return 0.0f;
				}
			}
			/*
			* @brief converts NDC to USC
			*
			*  !DEPRECATED! a new function in next releases
			*
			* @param NDC - the value to be converted in NDC
			* @param ratio - the screen ratio
			* @param xy - if its the x - Width or y - Height (in means of coordinates)
			*/
			static inline float USC_u(float NDC, ScreenRatio ratio, WidthHeightEnum xyz)
			{
				uint8_t denominator;
				if (ratio.width > ratio.height) {
					denominator = ratio.width;
				}
				else {
					denominator = ratio.height;
				}
				if (xyz == WidthHeightEnum::Width || xyz == WidthHeightEnum::Depth) {
					return NDC * 100 * (ratio.width * pow(10, countDigit(denominator) - 1));
				}
				if (xyz == WidthHeightEnum::Height) {
					return NDC * 100 * (ratio.height * pow(10, countDigit(denominator) - 1));
				}
			}
		};
	}
}