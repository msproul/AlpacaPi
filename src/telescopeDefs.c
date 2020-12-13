//*****************************************************************************
//focuser
//*****************************************************************************
enum
{
	kCmd_Focuser_absolute	=	0,		//*	Indicates whether the focuser is capable of absolute position.
	kCmd_Focuser_ismoving,				//*	Indicates whether the focuser is currently moving.
	kCmd_Focuser_maxincrement,			//*	Returns the focuser's maximum increment size.
	kCmd_Focuser_maxstep,				//*	Returns the focuser's maximum step size.
	kCmd_Focuser_position,				//*	Returns the focuser's current position.
	kCmd_Focuser_stepsize,				//*	Returns the focuser's step size.
	kCmd_Focuser_tempcomp,				//*	Retrieves the state of temperature compensation mode
//	kCmd_Focuser_tempcomp,				//*	Sets the device's temperature compensation mode.
	kCmd_Focuser_tempcompavailable,		//*	Indicates whether the focuser has temperature compensation.
	kCmd_Focuser_temperature,			//*	Returns the focuser's current temperature.
	kCmd_Focuser_halt,					//*	Immediatley stops focuser motion.
	kCmd_Focuser_move					//*	Moves the focuser to a new position.
};

//*****************************************************************************
//Rotator Specific Methods
//*****************************************************************************
enum
{
	kCmd_Rotator_canreverse=	0,		//*	Indicates whether the Rotator supports the Reverse method.
	kCmd_Rotator_ismoving,				//*	Indicates whether the rotator is currently moving.
	kCmd_Rotator_position,				//*	Returns the rotator's current position.
	kCmd_Rotator_reverse,				//*	Returns the rotator's Reverse state.
										//*	Sets the rotator's Reverse state.
	kCmd_Rotator_stepsize,				//*	Returns the minimum StepSize
	kCmd_Rotator_targetposition,		//*	Returns the destination position angle.
	kCmd_Rotator_halt,					//*	Immediatley stops rotator motion.
	kCmd_Rotator_move,					//*	Moves the rotator to a new relative position.
	kCmd_Rotator_moveabsolute,			//*	Moves the rotator to a new absolute position.
};

//static TYPE_CmdEntry	gRotatorCmdTable[]	=

//*****************************************************************************
//*	Telescope Specific Methods
//*****************************************************************************
enum
{

	kCmd_Telescope_alignmentmode	=	0,	//*	Returns the current mount alignment mode
	kCmd_Telescope_altitude,				//*	Returns the mount's Altitude above the horizon.
	kCmd_Telescope_aperturearea,			//*	Returns the telescope's aperture.
	kCmd_Telescope_aperturediameter,		//*	Returns the telescope's effective aperture.
	kCmd_Telescope_athome,					//*	Indicates whether the mount is at the home position.
	kCmd_Telescope_atpark,					//*	Indicates whether the telescope is at the park position.
	kCmd_Telescope_azimuth,					//*	Returns the telescope's aperture.
	kCmd_Telescope_canfindhome,				//*	Indicates whether the mount can find the home position.
	kCmd_Telescope_canpark,					//*	Indicates whether the telescope can be parked.
	kCmd_Telescope_canpulseguide,			//*	Indicates whether the telescope can be pulse guided.
	kCmd_Telescope_cansetdeclinationrate,	//*	Indicates whether the DeclinationRate property can be changed.
	kCmd_Telescope_cansetguiderates,		//*	Indicates whether the DeclinationRate property can be changed.
	kCmd_Telescope_cansetpark,				//*	Indicates whether the telescope park position can be set.
	kCmd_Telescope_cansetpierside,			//*	Indicates whether the telescope SideOfPier can be set.
	kCmd_Telescope_cansetrightascensionrate,//*	Indicates whether the RightAscensionRate property can be changed.
	kCmd_Telescope_cansettracking,			//*	Indicates whether the Tracking property can be changed.
	kCmd_Telescope_canslew,					//*	Indicates whether the telescope can slew synchronously.
	kCmd_Telescope_canslewaltaz,			//*	Indicates whether the telescope can slew synchronously to AltAz coordinates.
	kCmd_Telescope_canslewaltazasync,		//*	Indicates whether the telescope can slew asynchronously to AltAz coordinates.
	kCmd_Telescope_canslewasync,			//*	Indicates whether the telescope can slew asynchronously.
	kCmd_Telescope_cansync,					//*	Indicates whether the telescope can sync to equatorial coordinates.
	kCmd_Telescope_cansyncaltaz,			//*	Indicates whether the telescope can sync to local horizontal coordinates.
	kCmd_Telescope_declination,				//*	Returns the telescope's declination.
	kCmd_Telescope_declinationrate,			//*	Returns the telescope's declination tracking rate.
//	kCmd_Telescope_declinationrate,			//*	Sets the telescope's declination tracking rate.
	kCmd_Telescope_doesrefraction,			//*	Indicates whether atmospheric refraction is applied to coordinates.
//	kCmd_Telescope_doesrefraction,			//*	Determines whether atmospheric refraction is applied to coordinates.
	kCmd_Telescope_equatorialsystem,		//*	Returns the current equatorial coordinate system used by this telescope.
	kCmd_Telescope_focallength,				//*	Returns the telescope's focal length in meters.
	kCmd_Telescope_guideratedeclination,	//*	Returns the current Declination rate offset for telescope guiding
//	kCmd_Telescope_guideratedeclination,	//*	Sets the current Declination rate offset for telescope guiding.
	kCmd_Telescope_guideraterightascension,	//*	Returns the current RightAscension rate offset for telescope guiding
//	kCmd_Telescope_guideraterightascension,	//*	Sets the current RightAscension rate offset for telescope guiding.
	kCmd_Telescope_ispulseguiding,			//*	Indicates whether the telescope is currently executing a PulseGuide command
	kCmd_Telescope_rightascension,			//*	Returns the telescope's right ascension coordinate.
	kCmd_Telescope_rightascensionrate,		//*	Returns the telescope's right ascension tracking rate.
//	kCmd_Telescope_rightascensionrate,		//*	Sets the telescope's right ascension tracking rate.
	kCmd_Telescope_sideofpier,				//*	Returns the mount's pointing state.
//	kCmd_Telescope_sideofpier,				//*	Sets the mount's pointing state.
	kCmd_Telescope_siderealtime,			//*	Returns the local apparent sidereal time.
	kCmd_Telescope_siteelevation,			//*	Returns the observing site's elevation above mean sea level.
//	kCmd_Telescope_siteelevation,			//*	Sets the observing site's elevation above mean sea level.
	kCmd_Telescope_sitelatitude,			//*	Returns the observing site's latitude.
//	kCmd_Telescope_sitelatitude,			//*	Sets the observing site's latitude.
	kCmd_Telescope_sitelongitude,			//*	Returns the observing site's longitude.
//	kCmd_Telescope_sitelongitude,			//*	Sets the observing site's longitude.
	kCmd_Telescope_slewing,					//*	Indicates whether the telescope is currently slewing.
	kCmd_Telescope_slewsettletime,			//*	Returns the post-slew settling time.
//	kCmd_Telescope_slewsettletime,			//*	Sets the post-slew settling time.
	kCmd_Telescope_targetdeclination,		//*	Returns the current target declination.
//	kCmd_Telescope_targetdeclination,		//*	Sets the target declination of a slew or sync.
	kCmd_Telescope_targetrightascension,	//*	Returns the current target right ascension.
//	kCmd_Telescope_targetrightascension,	//*	Sets the target right ascension of a slew or sync.
	kCmd_Telescope_tracking,				//*	Indicates whether the telescope is tracking.
//	kCmd_Telescope_tracking,				//*	Enables or disables telescope tracking.
	kCmd_Telescope_trackingrate,			//*	Returns the current tracking rate.
//	kCmd_Telescope_trackingrate,			//*	Sets the mount's tracking rate.
	kCmd_Telescope_trackingrates,			//*	Returns a collection of supported DriveRates values.
	kCmd_Telescope_utcdate,					//*	Returns the UTC date/time of the telescope's internal clock.
//	kCmd_Telescope_utcdate,					//*	Sets the UTC date/time of the telescope's internal clock.
	kCmd_Telescope_abortslew,				//*	Immediatley stops a slew in progress.
	kCmd_Telescope_axisrates,				//*	Returns the rates at which the telescope may be moved about the specified axis.
	kCmd_Telescope_canmoveaxis,				//*	Indicates whether the telescope can move the requested axis.
	kCmd_Telescope_destinationsideofpier,	//*	Predicts the pointing state after a German equatorial mount slews to given coordinates.
	kCmd_Telescope_findhome,				//*	Moves the mount to the "home" position.
	kCmd_Telescope_moveaxis,				//*	Moves a telescope axis at the given rate.
	kCmd_Telescope_park,					//*	Park the mount
	kCmd_Telescope_pulseguide,				//*	Moves the scope in the given direction for the given time.
	kCmd_Telescope_setpark,					//*	Sets the telescope's park position
	kCmd_Telescope_slewtoaltaz,				//*	Synchronously slew to the given local horizontal coordinates.
	kCmd_Telescope_slewtoaltazasync,		//*	Asynchronously slew to the given local horizontal coordinates.
	kCmd_Telescope_slewtocoordinates,		//*	Synchronously slew to the given equatorial coordinates.
	kCmd_Telescope_slewtocoordinatesasync,	//*	Asynchronously slew to the given equatorial coordinates.
	kCmd_Telescope_slewtotarget,			//*	Synchronously slew to the TargetRightAscension and TargetDeclination coordinates.
	kCmd_Telescope_slewtotargetasync,		//*	Asynchronously slew to the TargetRightAscension and TargetDeclination coordinates.
	kCmd_Telescope_synctoaltaz,				//*	Syncs to the given local horizontal coordinates.
	kCmd_Telescope_synctocoordinates,		//*	Syncs to the given equatorial coordinates.
	kCmd_Telescope_synctotarget,			//*	Syncs to the TargetRightAscension and TargetDeclination coordinates.
	kCmd_Telescope_unpark					//*	Unparks the mount.
};


