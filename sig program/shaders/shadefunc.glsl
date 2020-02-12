
vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3 la, vec3 ld, vec3 ls, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha )
{
	// CamDev: revise use of lp-p or p:
	//vec3 l = normalize ( lp-p );      // light direction from p
	//vec3 v = normalize ( vec3(0,0,1)-p ); // viewer vector from p
	vec3 l = normalize ( lp );          // light direction from p
	vec3 v = normalize ( vec3(0,0,1) ); // viewer vector from p
	vec3 r = reflect ( -l, n );         // the reflected light ray
	//vec3 r = normalize(l+v);  // testing the half-vector instead of reflected ray

	float dotln = dot(l,n);
	vec3 amb = la * ka;
	vec3 dif = ld * kd * max(dotln,0.0); // lambertian component
	vec3 spe = max(dotln,0.0) * ls * ks * pow ( max(dot(v,r),0.0), sh );
	return vec4 ( amb + dif + spe + emi, alpha );
}
