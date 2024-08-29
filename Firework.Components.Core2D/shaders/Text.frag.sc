$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_characterData, 0);

uniform vec4 u_characterOffsetAndConstantData;
#define u_characterOffset_beg int(u_characterOffsetAndConstantData.x)
#define u_characterOffset_size int(u_characterOffsetAndConstantData.y)
#define u_typefaceGlyphInitPoint u_characterOffsetAndConstantData.z
#define u_typefaceGlyphLineSegmentLinear u_characterOffsetAndConstantData.w

uniform vec4 u_characterGlyphMetricsData;
#define u_characterGlyphMetric_ascent u_characterGlyphMetricsData.x
#define u_characterGlyphMetric_descent u_characterGlyphMetricsData.y
#define u_characterGlyphMetric_xInit u_characterGlyphMetricsData.z
#define u_characterGlyphMetric_advance u_characterGlyphMetricsData.w

uniform vec4 u_postProcessingData;
#define u_width u_postProcessingData.x
#define u_height u_postProcessingData.y
#define u_antialiasing int(u_postProcessingData.z)

float isPixelColored(float x, float y)
{
    int crossings = 0;

    for (int i = u_characterOffset_beg; i < u_characterOffset_beg + u_characterOffset_size - 1; i++)
    {
        if (texelFetch(s_characterData, ivec2(0, i + 1), 0).z == u_typefaceGlyphInitPoint)
            continue;

        if (texelFetch(s_characterData, ivec2(0, i + 1), 0).z == u_typefaceGlyphLineSegmentLinear)
        {
            vec2 p0 = texelFetch(s_characterData, ivec2(0, i), 0).xy;
            vec2 p1 = texelFetch(s_characterData, ivec2(0, i + 1), 0).xy;

            // y = m(x - x0) + y0
            // => x = (y - y0) * 1 / m + x0, m = (y1 - y0) / (x1 - x0)
            float xInt = (y - p0.y) * (p1.x - p0.x) / (p1.y - p0.y) + p0.x;
            if (x >= xInt && ((p0.y <= y && y < p1.y) || (p1.y <= y && y < p0.y)))
                ++crossings;
        }
        else
        {
            vec2 p0 = texelFetch(s_characterData, ivec2(0, i), 0).xy;
            vec2 p1 = texelFetch(s_characterData, ivec2(0, i + 1), 0).zw; // Control point.
            vec2 p2 = texelFetch(s_characterData, ivec2(0, i + 1), 0).xy;

            // x(t) = (1 - t)^2 x0 + 2t(1 - t)x1 + t^2 x2
            // y(t) = (1 - t)^2 y0 + 2t(1 - t)y1 + t^2 y2
            // so we have for t in with y(t) = y, then find x(t).
            // y = (1 - 2t + t^2)y0 + (2t - 2t^2)y1 + t^2 y2
            // => (y0 - 2y1 + y2)t^2 + 2t(y1 - y0) + y0 - y = 0
            // With high school maths (+ UNSW MATH2089 Numerical Methods catastrohic cancellation knowledge),
            // t = (-b +- sqrt(b^2 - 4ac)) / (2a), a = y0 - 2y1 + y2, b = 2(y1 - y0), c = y0 - y
            //   = (4ac) / (2a(-b -+ sqrt(b^2 - 4ac)))
            //   = 2c / (-b -+ sqrt(b^2 - 4ac))
            float a = p0.y - 2.0 * p1.y + p2.y;
            float b = 2.0 * (p1.y - p0.y);
            float c = p0.y - y;

            if (a == 0.0)
            {
                float t = -c / b;
                if (0.0 <= t && t < 1.0)
                {
                    float xInt = (1.0 - t) * (1.0 - t) * p0.x + 2.0 * t * (1.0 - t) * p1.x + t * t * p2.x;
                    if (x > xInt)
                        ++crossings;
                }
            }
            else if (b * b - 4.0 * a * c == 0.0)
            {
                float t = -b / (2.0 * a);
                if (0.0 <= t && t < 1.0)
                {
                    float xInt = (1.0 - t) * (1.0 - t) * p0.x + 2.0 * t * (1.0 - t) * p1.x + t * t * p2.x;
                    if (x > xInt)
                        ++crossings;
                }
            }
            else if (b * b - 4.0 * a * c > 0.0)
            {
                float t1 = (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a);
                float t2 = (2.0 * c) / (-b - sqrt(b * b - 4.0 * a * c));

                if (0.0 <= t1 && t1 < 1.0)
                {
                    float xInt = (1.0 - t1) * (1.0 - t1) * p0.x + 2.0 * t1 * (1.0 - t1) * p1.x + t1 * t1 * p2.x;
                    if (x > xInt)
                        ++crossings;
                }
                if (0.0 <= t2 && t2 < 1.0)
                {
                    float xInt = (1.0 - t2) * (1.0 - t2) * p0.x + 2.0 * t2 * (1.0 - t2) * p1.x + t2 * t2 * p2.x;
                    if (x > xInt)
                        ++crossings;
                }
            }
        }
    }

    return float(crossings % 2);
}
vec4 antiAliasedColor(float x, float y, float xRatio, float yRatio)
{
    float ret = 0.0;
    for (int j = -u_antialiasing; j <= u_antialiasing; j++)
    {
        for (int i = -u_antialiasing; i <= u_antialiasing; i++)
        {
            float shouldColor = isPixelColored(x + float(i) * 2.0 / (float(u_antialiasing) * 2.0 + 1.0) * xRatio / 2.0, y + (float(j) * 2.0) / (float(u_antialiasing) * 2.0 + 1.0) * yRatio / 2.0);
            ret += shouldColor;
        }
    }
    ret /= float((u_antialiasing * 2 + 1) * (u_antialiasing * 2 + 1));
    return vec4(1.0, 1.0, 1.0, ret);
}

// FIXME: Doesn't work in OpenGL mode.
void main()
{
    float x = v_texcoord0.x * (u_characterGlyphMetric_advance - u_characterGlyphMetric_xInit) + u_characterGlyphMetric_xInit;
    float y = v_texcoord0.y * (u_characterGlyphMetric_ascent - u_characterGlyphMetric_descent) + u_characterGlyphMetric_descent;
    float xRatio = (u_characterGlyphMetric_advance - u_characterGlyphMetric_xInit) / u_width;
    float yRatio = (u_characterGlyphMetric_ascent - u_characterGlyphMetric_descent) / u_height;
    
    vec4 color = antiAliasedColor(x, y, xRatio, yRatio);
    if (color.w == 0.0)
        discard;
    else gl_FragColor = color;
}