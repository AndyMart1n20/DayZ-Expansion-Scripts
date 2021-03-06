modded class DayZPlayerCamera3rdPersonVehicle
{   
	protected vector m_PreviousOrientation;
	protected bool m_WasFreeLook;

	protected float m_UpDownAngle;

	protected vector m_ExLagOffsetPosition;
	protected vector m_ExLagOffsetOrientation;
	protected float m_ExLagOffsetVelocityX[1];
	protected float m_ExLagOffsetVelocityY[1];
	protected float m_ExLagOffsetVelocityZ[1];
	protected float m_ExLagOffsetVelocityYaw[1];
	protected float m_ExLagOffsetVelocityPitch[1];
	protected float m_ExLagOffsetVelocityRoll[1];

	void DayZPlayerCamera3rdPersonVehicle( DayZPlayer pPlayer, HumanInputController pInput )
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("DayZPlayerCamera3rdPersonVehicle::DayZPlayerCamera3rdPersonVehicle - Start");
		#endif
		
		if ( m_pPlayer )
		{
			CarScript vehicle;
			ExpansionVehicleScript expVehicle;
			if ( Class.CastTo( vehicle, m_pPlayer.GetParent() ) )
			{
				if ( vehicle.IsInherited( ExpansionHelicopterScript ) )
				{
					m_fDistance 			= 12.0;
					m_CameraOffsetMS		= "0.0 1.0 0.0";	
					m_PreviousOrientation	= vehicle.GetOrientation();
				} else if ( vehicle.IsInherited( ExpansionPlaneScript ) )
				{
					m_fDistance 			= vehicle.GetCameraDistance();
					m_CameraOffsetMS		= "0.0 2.0 0.0";	
				} else if ( vehicle.IsInherited( ExpansionBoatScript ) )
				{
					m_fDistance 			= vehicle.GetCameraDistance();
					m_CameraOffsetMS		= Vector( 0, vehicle.GetCameraHeight(), 0 );
				} else if ( vehicle.IsInherited( ExpansionBulldozerScript ) )
				{
					m_fDistance 			= 6.0;
					m_CameraOffsetMS		= "0.0 1.5 0.0";	
				} else if ( vehicle.IsInherited( ExpansionBus ) )
				{
					m_fDistance 			= 12.0;
					m_CameraOffsetMS		= "0.0 1.5 0.0";	
				} else 
				{
					m_fDistance 			= 4.0;
					m_CameraOffsetMS		= "0.0 1.3 0.0";
				}
			} else if ( Class.CastTo( expVehicle, m_pPlayer.GetParent() ) )
			{
				m_fDistance 				= expVehicle.GetCameraDistance();
				m_CameraOffsetMS			= Vector( 0, expVehicle.GetCameraHeight(), 0 );
			}
		} else 
		{
			m_fDistance 		= 4.0;
			m_CameraOffsetMS	= "0.0 1.3 0.0";
		}
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("DayZPlayerCamera3rdPersonVehicle::DayZPlayerCamera3rdPersonVehicle - End");
		#endif
	}
	
	override void OnActivate( DayZPlayerCamera pPrevCamera, DayZPlayerCameraResult pPrevCameraResult )
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("DayZPlayerCamera3rdPersonVehicle::OnActivate - Start");
		#endif
		
		super.OnActivate( pPrevCamera, pPrevCameraResult );
		
		m_ExLagOffsetPosition = vector.Zero;
		m_ExLagOffsetVelocityX[0] = 0;
		m_ExLagOffsetVelocityY[0] = 0;
		m_ExLagOffsetVelocityZ[0] = 0;
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("DayZPlayerCamera3rdPersonVehicle::OnActivate - End");
		#endif
	}

	override void OnUpdate( float pDt, out DayZPlayerCameraResult pOutResult )
	{		
		#ifdef EXPANSIONEXPRINT
		EXPrint("DayZPlayerCamera3rdPersonVehicle::OnUpdate - Start");
		#endif
		
		super.OnUpdate( pDt, pOutResult );

		PlayerBase.Cast( m_pPlayer ).SetHeadInvisible( false );

		EntityAI entParent;
		if ( !Class.CastTo( entParent, m_pPlayer.GetParent() ) )
		{
			return;
		}

		ExpansionHelicopterScript helicopter;
		ExpansionVehicleScript vehicle;

		bool freeLookKey = false;
		float cameraHeight;
		float cameraDistance;

		if ( Class.CastTo( helicopter, m_pPlayer.GetParent() ) )
		{
			freeLookKey = helicopter.IsFreeLook();
			cameraHeight = helicopter.GetCameraHeight();
			cameraDistance = helicopter.GetCameraDistance();
		} else if ( Class.CastTo( vehicle, m_pPlayer.GetParent() ) )
		{
			freeLookKey = vehicle.IsFreeLook();
			cameraHeight = vehicle.GetCameraHeight();
			cameraDistance = vehicle.GetCameraDistance();
		} else
		{
			return;
		}

		// vehicle.FilterNextTrace();

		vector playerTransformWS[4];
		m_pPlayer.GetTransform( playerTransformWS );

		vector vehiclePositionWS = entParent.GetOrigin();
		vector vehiclePositionMS = vehiclePositionWS.InvMultiply4( playerTransformWS );
		vector cameraPosition = vehiclePositionMS + m_CameraOffsetMS;

		vector newOrientation = entParent.GetOrientation();
		vector orientDiff = vector.Zero;

		orientDiff[0] = Math.DiffAngle( m_PreviousOrientation[0], newOrientation[0] );
		orientDiff[1] = Math.DiffAngle( m_PreviousOrientation[1], newOrientation[1] );
		orientDiff[2] = Math.DiffAngle( m_PreviousOrientation[2], newOrientation[2] );

		vector posDiffWS = GetVelocity( entParent ) * pDt;
		vector posDiffLS = posDiffWS.InvMultiply3( playerTransformWS );

		vector rotation = vector.Zero;

		//! smooth it!
		m_ExLagOffsetPosition[0] = Math.SmoothCD( m_ExLagOffsetPosition[0], posDiffLS[0], m_ExLagOffsetVelocityX, 0.4, 1000, pDt );
		m_ExLagOffsetPosition[1] = Math.SmoothCD( m_ExLagOffsetPosition[1], posDiffLS[1], m_ExLagOffsetVelocityY, 0.4, 1000, pDt );
		m_ExLagOffsetPosition[2] = Math.SmoothCD( m_ExLagOffsetPosition[2], posDiffLS[2], m_ExLagOffsetVelocityZ, 0.4, 1000, pDt );

		m_ExLagOffsetOrientation[1] = Math.SmoothCD( m_ExLagOffsetOrientation[1], -newOrientation[1], m_ExLagOffsetVelocityPitch, 0.3, 1000, pDt );

		bool freeLookSettings = GetExpansionClientSettings().UseCameraLock;
			
		if ( ( freeLookSettings && !freeLookKey ) )
		{
			m_fLeftRightAngle = 0;

			rotation[0] = m_fLeftRightAngle;
			rotation[1] = m_ExLagOffsetOrientation[1];

			Math3D.YawPitchRollMatrix( rotation, pOutResult.m_CameraTM );
		}
			
		pOutResult.m_CameraTM[3] = cameraPosition + Vector( 0, cameraHeight, 0 ) - m_ExLagOffsetPosition;

		pOutResult.m_fIgnoreParentRoll		= 1.0;
		pOutResult.m_fInsideCamera		  = 0.0;
		pOutResult.m_iDirectBone			= -1.0;
		pOutResult.m_fUseHeading			= 0.0;
		pOutResult.m_fDistance			  = cameraDistance;
		pOutResult.m_fPositionModelSpace	= 1.0;

		m_PreviousOrientation = newOrientation;
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("DayZPlayerCamera3rdPersonVehicle::OnUpdate - End");
		#endif
	}
}