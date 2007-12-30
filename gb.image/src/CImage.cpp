/***************************************************************************

  CImage.cpp

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CIMAGE_CPP

#include <math.h>

#include "gambas.h"
#include "kimageeffect.h"
#include "effect.h"
#include "main.h"

#include "CImage.h"


static void return_copy(GB_IMAGE img)
{
	GB_IMAGE copy;
	GB_IMAGE_INFO info;

	GB.Image.Info(img, &info);
	GB.Image.Create(&copy, info.data, info.width, info.height, info.format);
	GB.ReturnObject(copy);
}

BEGIN_METHOD(CIMAGE_gradient, GB_INTEGER w; GB_INTEGER h; GB_INTEGER cs; GB_INTEGER cd; GB_INTEGER orient; GB_INTEGER x; GB_INTEGER y)

	QSize size(VARG(w), VARG(h));
	QColor cs(VARG(cs));
	QColor cd(VARG(cd));

	if (MISSING(x) || MISSING(y))
	{
		QImage img = KImageEffect::gradient(size, cs, cd, (KImageEffect::GradientType)VARG(orient));
		GB.ReturnObject(img.object());
	}
	else
	{
		QImage img = KImageEffect::unbalancedGradient(size, cs, cd, (KImageEffect::GradientType)VARG(orient), VARG(x), VARG(y));
		GB.ReturnObject(img.object());
	}

END_METHOD


BEGIN_METHOD(CIMAGE_intensity, GB_FLOAT val; GB_INTEGER channel)

	KImageEffect::RGBComponent channel = KImageEffect::All;
	QImage img(THIS);

	if (!MISSING(channel))
		channel = (KImageEffect::RGBComponent)VARG(channel);

	if (channel == KImageEffect::All)
		KImageEffect::intensity(img, VARG(val));
	else
		KImageEffect::channelIntensity(img, VARG(val), channel);

END_METHOD


BEGIN_METHOD(CIMAGE_flatten, GB_INTEGER c1; GB_INTEGER c2)

	QImage img(THIS);

	KImageEffect::flatten(img, VARG(c1), VARG(c2));

END_METHOD


BEGIN_METHOD(CIMAGE_fade, GB_INTEGER col; GB_FLOAT val)

	QImage img(THIS);

	KImageEffect::fade(img, VARG(val), VARG(col));

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_gray)

	QImage img(THIS);

	KImageEffect::toGray(img, false);

END_METHOD


BEGIN_METHOD(CIMAGE_desaturate, GB_FLOAT val)

	QImage img(THIS);

	KImageEffect::desaturate(img, VARG(val));

END_METHOD


BEGIN_METHOD(CIMAGE_threshold, GB_FLOAT val)

	QImage img(THIS);

	KImageEffect::threshold(img, (uint)(VARG(val) * 255));

END_METHOD


BEGIN_METHOD(CIMAGE_solarize, GB_FLOAT val)

	QImage img(THIS);

	KImageEffect::solarize(img, VARG(val) * 100);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_normalize)

	QImage img(THIS);

	KImageEffect::normalize(img);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_equalize)

	QImage img(THIS);

	KImageEffect::equalize(img);

END_METHOD


BEGIN_METHOD(CIMAGE_balance, GB_FLOAT brightness; GB_FLOAT contrast; GB_FLOAT gamma; GB_INTEGER channel)

	Effect::balance(THIS, VARGOPT(channel, Effect::All), (int)(VARG(brightness) * 50), (int)(VARG(contrast) * 50), (int)(VARG(gamma) * 50));

END_METHOD


BEGIN_METHOD(CIMAGE_invert, GB_INTEGER channel)

	Effect::invert(THIS, VARGOPT(channel, Effect::All));

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_emboss)

	QImage src(THIS);
	QImage dest = KImageEffect::emboss(src, 0, 1);
	GB.ReturnObject(dest.object());

END_METHOD


BEGIN_METHOD(CIMAGE_edge, GB_FLOAT radius)

	double radius = VARGOPT(radius, -1);

	if (radius != 0)
	{
		if (radius < 0)
			radius = 0;
		QImage src(THIS);
		QImage dest = KImageEffect::edge(src, radius);
		GB.ReturnObject(dest.object());
	}
	else
		return_copy(THIS);

END_METHOD


BEGIN_METHOD_VOID(CIMAGE_despeckle)

	QImage src(THIS);
	QImage dest = KImageEffect::despeckle(src);
	GB_IMAGE img = dest.object();

	GB.ReturnObject(img);

END_METHOD


BEGIN_METHOD(CIMAGE_blur, GB_FLOAT val)

	double val = VARGOPT(val, 0.2);
  double sigma = 0.5 + val * (4.0 - 0.5);
	double radius = 8;

	QImage src(THIS);
	QImage dest = KImageEffect::blur(src, radius, sigma);
	GB.ReturnObject(dest.object());

END_METHOD


BEGIN_METHOD(CIMAGE_sharpen, GB_FLOAT val)

	double val = VARGOPT(val, 0.2);
	double radius = 0.1 + val * (2.5 - 0.1);
  double sigma = radius < 1 ? radius : sqrt(radius);

	QImage src(THIS);
	QImage dest = KImageEffect::sharpen(src, radius, sigma);
  GB.ReturnObject(dest.object());

END_METHOD


BEGIN_METHOD(CIMAGE_charcoal, GB_FLOAT radius)

	double radius = VARGOPT(radius, -1);

	if (radius != 0)
	{
		if (radius < 0)
			radius = 0;
		QImage src(THIS);
		QImage dest = KImageEffect::charcoal(src, radius, 0.5);
		GB.ReturnObject(dest.object());
	}
	else
		return_copy(THIS);

END_METHOD


BEGIN_METHOD(CIMAGE_oil_paint, GB_FLOAT radius)

	double radius = VARGOPT(radius, -1);

	if (radius != 0)
	{
		if (radius < 0)
			radius = 0;
		QImage src(THIS);
		QImage dest = KImageEffect::oilPaintConvolve(src, radius);
		GB.ReturnObject(dest.object());
	}
	else
		return_copy(THIS);

END_METHOD


BEGIN_METHOD(CIMAGE_spread, GB_INTEGER amount)

	int amount = VARGOPT(amount, 0);

	if (amount > 0)
	{
		QImage src(THIS);
		QImage dest = KImageEffect::spread(src, VARGOPT(amount, 3));
		GB.ReturnObject(dest.object());
	}
	else
		return_copy(THIS);

END_METHOD


BEGIN_METHOD(CIMAGE_shade, GB_FLOAT azimuth; GB_FLOAT elevation)

	QImage src(THIS);
	QImage dest = KImageEffect::shade(src, true, VARGOPT(azimuth, 30.0), VARGOPT(elevation, 30.0));
	GB.ReturnObject(dest.object());

END_METHOD


BEGIN_METHOD(CIMAGE_swirl, GB_FLOAT angle; GB_INTEGER background)

	QImage src(THIS);
	QImage dest = KImageEffect::swirl(src, VARGOPT(angle, 50.0), VARGOPT(background, 0xFFFFFF) ^ 0xFF000000);
	GB.ReturnObject(dest.object());

END_METHOD


BEGIN_METHOD(CIMAGE_wave, GB_FLOAT amp; GB_FLOAT freq; GB_INTEGER background)

	QImage src(THIS);
	QImage dest = KImageEffect::wave(src, VARGOPT(amp, 25.0), VARGOPT(freq, 150.0), VARGOPT(background, 0xFFFFFF) ^ 0xFF000000);
	GB.ReturnObject(dest.object());

END_METHOD


BEGIN_METHOD(CIMAGE_noise, GB_INTEGER type)

	QImage src(THIS);
	QImage dest = KImageEffect::addNoise(src, (KImageEffect::NoiseType)VARG(type));
	GB.ReturnObject(dest.object());

END_METHOD


BEGIN_METHOD(CIMAGE_implode, GB_FLOAT factor; GB_INTEGER background)

	QImage src(THIS);
	QImage dest = KImageEffect::implode(src, VARGOPT(factor, 1.0) * 100.0, VARGOPT(background, 0xFFFFFF) ^ 0xFF000000);
	GB.ReturnObject(dest.object());

END_METHOD


GB_DESC CImageDesc[] =
{
  GB_DECLARE("Image", 0),

	GB_CONSTANT("Vertical", "i", KImageEffect::VerticalGradient),
	GB_CONSTANT("Horizontal", "i", KImageEffect::HorizontalGradient),
	GB_CONSTANT("Diagonal", "i", KImageEffect::DiagonalGradient),
	GB_CONSTANT("Cross", "i", KImageEffect::CrossDiagonalGradient),
	GB_CONSTANT("Pyramid", "i", KImageEffect::PyramidGradient),
	GB_CONSTANT("Rectangle", "i", KImageEffect::RectangleGradient),
	GB_CONSTANT("PipeCross", "i", KImageEffect::PipeCrossGradient),
	GB_CONSTANT("Elliptic", "i", KImageEffect::EllipticGradient),

	GB_CONSTANT("All", "i", KImageEffect::All),
	GB_CONSTANT("Red", "i", KImageEffect::Red),
	GB_CONSTANT("Green", "i", KImageEffect::Green),
	GB_CONSTANT("Blue", "i", KImageEffect::Blue),

	GB_CONSTANT("Uniform", "i", KImageEffect::UniformNoise),
	GB_CONSTANT("Gaussian", "i", KImageEffect::GaussianNoise),
	GB_CONSTANT("Multiplicative", "i", KImageEffect::MultiplicativeGaussianNoise),
  GB_CONSTANT("Impulse", "i", KImageEffect::ImpulseNoise),
	GB_CONSTANT("Laplacian", "i", KImageEffect::LaplacianNoise),
	GB_CONSTANT("Poisson", "i", KImageEffect::PoissonNoise),

	GB_STATIC_METHOD("Gradient", "Image", CIMAGE_gradient, "(Width)i(Height)i(SrcColor)i(DstColor)i(Orientation)i[(XDecay)i(YDecay)i]"),

	GB_METHOD("Intensity", NULL, CIMAGE_intensity, "(Value)f[(Channel)i]"),
	GB_METHOD("Flatten", NULL, CIMAGE_flatten, "(DarkColor)i(BrightColor)i"),
	GB_METHOD("Fade", NULL, CIMAGE_fade, "(Color)i(Value)f"),
	GB_METHOD("Gray", NULL, CIMAGE_gray, NULL),
	GB_METHOD("Desaturate", NULL, CIMAGE_desaturate, "(Value)f"),
	GB_METHOD("Threshold", NULL, CIMAGE_threshold, "(Value)f"),
	GB_METHOD("Solarize", NULL, CIMAGE_solarize, "(Value)f"),
	GB_METHOD("Normalize", NULL, CIMAGE_normalize, NULL),
	GB_METHOD("Equalize", NULL, CIMAGE_equalize, NULL),
	GB_METHOD("Balance", NULL, CIMAGE_balance, "(Brightness)f(Contrast)f(Gamma)f[(Channel)i]"),
	GB_METHOD("Invert", NULL, CIMAGE_invert, "[(Channel)i]"),

	GB_METHOD("Emboss", "Image", CIMAGE_emboss, NULL),
	GB_METHOD("Edge", "Image", CIMAGE_edge, "[(Radius)f]"),
	GB_METHOD("Despeckle", "Image", CIMAGE_despeckle, NULL),
	GB_METHOD("Charcoal", "Image", CIMAGE_charcoal, "[(Radius)f]"),
	GB_METHOD("OilPaint", "Image", CIMAGE_oil_paint, "[(Radius)f]"),
	GB_METHOD("Blur", "Image", CIMAGE_blur, "[(Value)f]"),
	GB_METHOD("Sharpen", "Image", CIMAGE_sharpen, "[(Value)f]"),
	GB_METHOD("Spread", "Image", CIMAGE_spread, "[(Amount)i]"),
	GB_METHOD("Shade", "Image", CIMAGE_shade, "[(Azimuth)f(Elevation)f]"),
	GB_METHOD("Swirl", "Image", CIMAGE_swirl, "[(Angle)f(Background)i]"),
	GB_METHOD("Wave", "Image", CIMAGE_wave, "[(Amplitude)f(WaveLength)f(Background)i]"),
	GB_METHOD("Noise", "Image", CIMAGE_noise, "(Noise)i"),
	GB_METHOD("Implode", "Image", CIMAGE_implode, "[(Factor)f(Background)i]"),

  GB_END_DECLARE
};

