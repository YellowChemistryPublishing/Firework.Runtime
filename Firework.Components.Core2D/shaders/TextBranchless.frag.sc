$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_characterData, 0);

uniform vec4 u_characterOffsetAndConstantData;
#define u_characterOffset_beg floatBitsToInt(u_characterOffsetAndConstantData.x)
#define u_characterOffset_size floatBitsToInt(u_characterOffsetAndConstantData.y)
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
#define u_antialiasing floatBitsToInt(u_postProcessingData.z)

float isPixelColored(float x, float y)
{
    int crossings = 0;

    for (int i = u_characterOffset_beg; i < u_characterOffset_beg + u_characterOffset_size - 1; i++)
    {
        if (texelFetch(s_characterData, ivec2(0, i + 1), 0).z == u_typefaceGlyphInitPoint)
            continue;

        int isLinear = int(texelFetch(s_characterData, ivec2(0, i + 1), 0).z == u_typefaceGlyphLineSegmentLinear);
        
        // Linear Things v

        vec2 p0l = texelFetch(s_characterData, ivec2(0, i), 0).xy;
        vec2 p1l = texelFetch(s_characterData, ivec2(0, i + 1), 0).xy;

        // y = m(x - x0) + y0
        // => x = (y - y0) * 1 / m + x0, m = (y1 - y0) / (x1 - x0)
        float xInt = (y - p0l.y) * (p1l.x - p0l.x) / (p1l.y - p0l.y) + p0l.x;
        int linearCrossing = int(x >= xInt && ((p0l.y <= y && y < p1l.y) || (p1l.y <= y && y < p0l.y)));

        // Linear Things ^ / v Quadratic Things

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

        #define tCaseLinear (-c / b)
        #define tCase1Sol (-b / (2.0 * a))
        #define tCase2Sol1 ((-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a))
        #define tCase2Sol2 ((2.0 * c) / (-b - sqrt(b * b - 4.0 * a * c)))

        int quadraticCrossings =
        //  | a = 0 => f(t) is linear.
        //  |           | t in [0, 1), domain validation.          | x > xInt: Raycasting.
        int(a == 0.0 && 0.0 <= tCaseLinear && tCaseLinear < 1.0 && x > (1.0 - tCaseLinear) * (1.0 - tCaseLinear) * p0.x + 2.0 * tCaseLinear * (1.0 - tCaseLinear) * p1.x + tCaseLinear * tCaseLinear * p2.x) +
        //  | b^2 - 4ac = 0 => f(t) is quadratic with one root.
        //  |                             | t in [0, 1), domain validation.      | x > xInt: Raycasting.
        int(b * b - 4.0 * a * c == 0.0 && 0.0 <= tCase1Sol && tCase1Sol < 1.0 && x > (1.0 - tCase1Sol) * (1.0 - tCase1Sol) * p0.x + 2.0 * tCase1Sol * (1.0 - tCase1Sol) * p1.x + tCase1Sol * tCase1Sol * p2.x) +
        //  | b^2 - 4ac > 0 => f(t) is quadratic with two roots.
        int(b * b - 4.0 * a * c > 0.0) *
        (
            //  | t in [0, 1), domain validation.        | x > xInt: Raycasting.
            int(0.0 <= tCase2Sol1 && tCase2Sol1 < 1.0 && x > (1.0 - tCase2Sol1) * (1.0 - tCase2Sol1) * p0.x + 2.0 * tCase2Sol1 * (1.0 - tCase2Sol1) * p1.x + tCase2Sol1 * tCase2Sol1 * p2.x) +
            //  | t in [0, 1), domain validation.        | x > xInt: Raycasting.
            int(0.0 <= tCase2Sol2 && tCase2Sol2 < 1.0 && x > (1.0 - tCase2Sol2) * (1.0 - tCase2Sol2) * p0.x + 2.0 * tCase2Sol2 * (1.0 - tCase2Sol2) * p1.x + tCase2Sol2 * tCase2Sol2 * p2.x)
        );

        // ^ Quadratic Things
        
        crossings += isLinear * linearCrossing + (1 - isLinear) * quadraticCrossings;
    }

    return float(crossings % 2);
}
vec4 antiAliasedColor(float x, float y, float xRatio, float yRatio)
{
    float ret = 0.0;
    if (u_antialiasing == 0)
    {
        float shouldColor = isPixelColored(x, y);
        ret = shouldColor;
    }
    else
    {
        for (int j = -u_antialiasing; j <= u_antialiasing; j++)
        {
            for (int i = -u_antialiasing; i <= u_antialiasing; i++)
            {
                float shouldColor = isPixelColored(x + float(i) / float(u_antialiasing) * xRatio / 2.0, y + float(j) / float(u_antialiasing) * yRatio / 2.0);
                ret += shouldColor;
            }
        }
        ret /= float((u_antialiasing * 2 + 1) * (u_antialiasing * 2 + 1));
    }
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