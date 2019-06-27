#ifndef INVIWO_PROJECTS_INTERPOLATION_H
#define INVIWO_PROJECTS_INTERPOLATION_H

#include "glm/glm.hpp"

void InterpolateCurveGlobalNoDeriv(const std::vector<glm::vec3>& InPoints, tinynurbs::Curve<3, float>& ResCurve);

void InterpolateCurve(const std::vector<glm::vec3>& InPoints, tinynurbs::Curve<3, float>& ResCurve);

void ApproximateCurve(const std::vector<glm::vec3>& InPoints, tinynurbs::Curve<3, float>& ResCurve);

//void SaveCurve(tinynurbs::Curve<3, float>& Curve, const char* FileName, int NumPoints);
//
//void SavePolyline(std::vector<glm::vec3>& Curve, const char* FileName);

#endif //INVIWO_PROJECTS_INTERPOLATION_H
