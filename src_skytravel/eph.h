
#ifdef __cplusplus
	extern "C" {
#endif

//*	sas -> side.angle.side
//*	sss -> side.side.side.


void eph(TYPE_Time*,TYPE_LatLon*,planet_struct*,sun_moon_struct*);
void sphsas(TYPE_SpherTrig *sphptr);
void sphsss(TYPE_SpherTrig *sphptr);

#ifdef __cplusplus
}
#endif
