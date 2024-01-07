//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "terrain/Fluid.h"
#include "dgl/dgl.h"

//==============================================================================
//  FUNCTIONS
//==============================================================================

void fluid::Render( bool& EyeSubmerged )
{
    f32 BaseDriftX, BaseDriftY;

    f32 Q1 = 1.0f /   48.0f;    // This just looks good.
    f32 Q2 = 1.0f / 2048.0f;    // This is the size of the terrain.

    f32 SBase[] = { Q1,  0, 0, 0 };
    f32 TBase[] = { 0,  Q1, 0, 0 };

    f32 SLMap[] = { Q2,  0, 0, 0 };
    f32 TLMap[] = { 0,  Q2, 0, 0 };

    // Several attributes in the fluid vary over time.  Get a definitive time
    // reading now to be used throughout this render pass.
    m_Seconds  = SECONDS - m_BaseSeconds;

    // Based on the view frustrum, accumulate the list of triangles that 
    // comprise the fluid surface for this render pass.
    RunQuadTree( EyeSubmerged );

    // Quick debug render.
#if 0
    if( 0 )
    {
        s32 i;
        for( i = 0; i < m_IUsed / 3; i++ )
        {
            glDisable   ( GL_TEXTURE_2D );
            glEnable    ( GL_BLEND );
            glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glBegin     ( GL_TRIANGLES );
            glColor4f   ( 0.0f, 1.0f, 0.0f, 0.5f );
            glVertex3f  ( m_pVertex[m_pIndex[i*3+0]].XYZ.X, m_pVertex[m_pIndex[i*3+0]].XYZ.Y, m_pVertex[m_pIndex[i*3+0]].XYZ.Z ); 
            glVertex3f  ( m_pVertex[m_pIndex[i*3+1]].XYZ.X, m_pVertex[m_pIndex[i*3+1]].XYZ.Y, m_pVertex[m_pIndex[i*3+1]].XYZ.Z ); 
            glVertex3f  ( m_pVertex[m_pIndex[i*3+2]].XYZ.X, m_pVertex[m_pIndex[i*3+2]].XYZ.Y, m_pVertex[m_pIndex[i*3+2]].XYZ.Z ); 
            glEnd       ();
        }
    }
#endif

    //
    // We need to compute some time dependant values before we start rendering.
    //

    // Base texture drift.
    {
        #define BASE_DRIFT_CYCLE_TIME    8.0f
        #define BASE_DRIFT_RATE          0.02f
        #define BASE_DRIFT_SCALAR        0.03f

        f32 Phase = FMOD( m_Seconds * (TWO_PI/BASE_DRIFT_CYCLE_TIME), TWO_PI );

        BaseDriftX = m_Seconds       * BASE_DRIFT_RATE;
        BaseDriftY = COSINE( Phase ) * BASE_DRIFT_SCALAR;
    }

    //--------------------------------------------------------------------------
    //--
    //-- Let's rock.
    //--

    //--------------------------------------------------------------------------
    //-- Debug - wire

    if( m_ShowWire )
    {
        glPolygonMode       ( GL_FRONT_AND_BACK, GL_LINE );
        glEnableClientState ( GL_VERTEX_ARRAY );
        glVertexPointer     ( 3, GL_FLOAT, sizeof(vertex), &(m_pVertex[0].XYZ) );
        glDisable           ( GL_TEXTURE_2D );
        glDisable           ( GL_DEPTH_TEST );         
        glEnable            ( GL_BLEND );
        glBlendFunc         ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glColor4f           ( 0.5f, 0.5f, 0.5f, 0.5f );
        glDrawElements      ( GL_TRIANGLES, m_IUsed, GL_UNSIGNED_SHORT, m_pIndex );
        glDisable           ( GL_BLEND );
        glEnable            ( GL_DEPTH_TEST );
        glColor4f           ( 1.0f, 1.0f, 0.0f, 1.0f );
        glDrawElements      ( GL_TRIANGLES, m_IUsed, GL_UNSIGNED_SHORT, m_pIndex );        
        glPolygonMode       ( GL_FRONT_AND_BACK, GL_FILL );
    }

    //--------------------------------------------------------------------------
    //-- Initializations for Phase 1 - base textures

    glPolygonMode       ( GL_FRONT_AND_BACK, GL_FILL );

    glEnableClientState ( GL_VERTEX_ARRAY );
    glVertexPointer     ( 3, GL_FLOAT, sizeof(vertex), &(m_pVertex[0].XYZ) );

    glEnable            ( GL_TEXTURE_2D );
    glEnable            ( GL_BLEND );

    glTexEnvi           ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glBlendFunc         ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glMatrixMode        ( GL_TEXTURE );
    glPushMatrix        ();
    glLoadIdentity      ();

    glEnableClientState ( GL_COLOR_ARRAY );

    glEnable            ( GL_TEXTURE_GEN_S ); 
    glEnable            ( GL_TEXTURE_GEN_T ); 
    glTexGeni           ( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
    glTexGeni           ( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
    glTexGenfv          ( GL_S, GL_OBJECT_PLANE, SBase );
    glTexGenfv          ( GL_T, GL_OBJECT_PLANE, TBase );

    glBindTexture       ( GL_TEXTURE_2D, m_BaseTexture.getGLName() );

    //--------------------------------------------------------------------------
    //-- Initializations for Phase 1a - first base texture

    glRotatef           ( 30.0f, 0.0f, 0.0f, 1.0f );
    glColorPointer      ( 4, GL_FLOAT, sizeof(vertex), &(m_pVertex[0].RGBA1a) );

	 glDepthMask(GL_FALSE);

    //--------------------------------------------------------------------------
    //-- Render Phase 1a - first base texture

    if( m_ShowBaseA )
    {
        glDrawElements  ( GL_TRIANGLES, m_IUsed, GL_UNSIGNED_SHORT, m_pIndex );
    }

    //--------------------------------------------------------------------------
    //-- Initializations for Phase 1b - second base texture

    glRotatef           ( 30.0f, 0.0f, 0.0f, 1.0f );
    glTranslatef        ( BaseDriftX, BaseDriftY, 0.0f );
    glColorPointer      ( 4, GL_FLOAT, sizeof(vertex), &(m_pVertex[0].RGBA1b) );

    //--------------------------------------------------------------------------
    //-- Render Phase 1b - first base texture

    if( m_ShowBaseB )
    {
        glDrawElements  ( GL_TRIANGLES, m_IUsed, GL_UNSIGNED_SHORT, m_pIndex );
    }

    //--------------------------------------------------------------------------
    //-- Cleanup from Phase 1 - base textures

    glDisableClientState( GL_COLOR_ARRAY  );
    glPopMatrix         ();

    //--------------------------------------------------------------------------
    //-- Initializations for Phase 2 - light map

    // glColor4f           ( 1.0f, 1.0f, 1.0f, 1.0f );
    // glBindTexture       ( GL_TEXTURE_2D, m_LightMapTexture.getGLName() );

    // glTexEnvi           ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    // glBlendFunc         ( GL_DST_COLOR, GL_ZERO );

    // glMatrixMode        ( GL_TEXTURE );
    // glPushMatrix        ();
    // glLoadIdentity      ();

    // glTexGenfv          ( GL_S, GL_OBJECT_PLANE, SLMap );
    // glTexGenfv          ( GL_T, GL_OBJECT_PLANE, TLMap );

    //--------------------------------------------------------------------------
    //-- Render Phase 2 - light map

    // if( m_ShowLightMap )
    // {
        // glDrawElements  ( GL_TRIANGLES, m_IUsed, GL_UNSIGNED_SHORT, m_pIndex );
    // }

    //--------------------------------------------------------------------------
    //-- Cleanup from Phase 2 - light map

    // glPopMatrix         ();
    glMatrixMode        ( GL_MODELVIEW );

    glDisable           ( GL_TEXTURE_GEN_S ); 
    glDisable           ( GL_TEXTURE_GEN_T ); 

    //--------------------------------------------------------------------------
    //-- Initializations for Phase 3 - environment map

    glEnableClientState ( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer   ( 2, GL_FLOAT, sizeof(vertex), &(m_pVertex[0].UV3) );

    glTexEnvi           ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glBlendFunc         ( GL_SRC_ALPHA, GL_ONE );

    glColor4f           ( 1.0f, 1.0f, 1.0f, m_EnvMapIntensity );
    glBindTexture       ( GL_TEXTURE_2D, m_EnvMapTexture.getGLName() );

    //--------------------------------------------------------------------------
    //-- Render Phase 3 - environment map / specular

    if( m_ShowEnvMap )
    {
        glDrawElements  ( GL_TRIANGLES, m_IUsed, GL_UNSIGNED_SHORT, m_pIndex );
    }

    //--------------------------------------------------------------------------
    //-- Initializations for Phase 4 - fog

    glDisable           ( GL_TEXTURE_2D );
    glBlendFunc         ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnableClientState ( GL_COLOR_ARRAY );
    glColorPointer      ( 4, GL_FLOAT, sizeof(vertex), &(m_pVertex[0].RGBA4) );

    glDisableClientState( GL_TEXTURE_COORD_ARRAY );

    //--------------------------------------------------------------------------
    //-- Render Phase 4 - fog

    if( m_ShowFog )
    {
        glDrawElements  ( GL_TRIANGLES, m_IUsed, GL_UNSIGNED_SHORT, m_pIndex );
    }

    //--------------------------------------------------------------------------
    //-- Cleanup from all Phases

	 glDepthMask(GL_TRUE);
    
    glDisable           ( GL_BLEND );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY  );

    //--------------------------------------------------------------------------
    //-- We're done with the rendering.
}

//==============================================================================
