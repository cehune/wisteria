//
//  utils.hpp
//  terrace
//
//  Created by celine on 2026-03-15.
//

#include "vec3.hpp"
#include <simd/simd.h>

float dot(const vec3& v, const vec3& u);
vec3 cross_product(const vec3& v, const vec3& u);
vec3 hadamard_product(const vec3& v, const vec3& u);

vec3 random_vector();
vec3 random_vector_bounded(double min, double max);
vec3 random_vector_unit();
vec3 random_vec_on_hemisphere(const vec3& normal);
vec3 random_unit_aperture_loc();

vec3 vec_max(const vec3& v, const vec3& u);
vec3 vec_min(const vec3& v, const vec3& u);
void print(const vec3h& u);
