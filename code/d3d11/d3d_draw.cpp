// D3D headers
#include "d3d_common.h"
#include "d3d_driver.h"
#include "d3d_wnd.h"
#include "d3d_state.h"
#include "d3d_image.h"
#include "d3d_shaders.h"

// @pjb: this is just here to deliberately fuck the build if driver is used in here
#define driver #driver_disallowed

void UpdateDirtyView()
{
    // Upload the constants
    d3dViewConstantBuffer_t* cb = QD3D::MapDynamicBuffer<d3dViewConstantBuffer_t>( g_pImmediateContext, g_DrawState.viewRenderData.constantBuffer );
    memcpy( cb, &g_RunState.constants, sizeof(d3dViewConstantBuffer_t) );
    g_pImmediateContext->Unmap( g_DrawState.viewRenderData.constantBuffer, 0 );
    g_RunState.dirtyConstants = qfalse;
}

// @pjb: forceinline because I don't want to put the 'if' inside UpdateDirtyTransform
__forceinline void UpdateViewState()
{
    // If we have dirty constants, update the constant buffer
    if ( g_RunState.dirtyConstants )
        UpdateDirtyView();

    // Select what state we want

}

void DrawQuad( 
    const d3dQuadRenderData_t* qrd,
    const d3dImage_t* image,
    const float* coords,
    const float* texcoords,
    const float* color )
{
    UpdateViewState();

    //
    // Update the constant buffer
    //
    d3dQuadRenderConstantBuffer_t* cb = QD3D::MapDynamicBuffer<d3dQuadRenderConstantBuffer_t>( 
        g_pImmediateContext, 
        qrd->constantBuffer );

    if ( color ) {
        memcpy( cb->color, color, sizeof(float) * 3 );
        cb->color[3] = 1;
    } else {
        cb->color[0] = 1; cb->color[1] = 1; cb->color[2] = 1; cb->color[3] = 1; 
    }

    g_pImmediateContext->Unmap( qrd->constantBuffer, 0 );

    //
    // Update the vertex buffer
    //
    d3dQuadRenderVertex_t* vb = QD3D::MapDynamicBuffer<d3dQuadRenderVertex_t>( 
        g_pImmediateContext, 
        qrd->vertexBuffer );

    vb[0].texcoords[0] = texcoords[0]; vb[0].texcoords[1] = texcoords[1];
    vb[1].texcoords[0] = texcoords[2]; vb[1].texcoords[1] = texcoords[1];
    vb[2].texcoords[0] = texcoords[2]; vb[2].texcoords[1] = texcoords[3];
    vb[3].texcoords[0] = texcoords[0]; vb[3].texcoords[1] = texcoords[3];

    vb[0].coords[0] = coords[0]; vb[0].coords[1] = coords[1];
    vb[1].coords[0] = coords[2]; vb[1].coords[1] = coords[1];
    vb[2].coords[0] = coords[2]; vb[2].coords[1] = coords[3];
    vb[3].coords[0] = coords[0]; vb[3].coords[1] = coords[3];

    g_pImmediateContext->Unmap( qrd->vertexBuffer, 0 );

    //
    // Draw
    //
    UINT stride = sizeof(float) * 4;
    UINT offset = 0;

    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &qrd->vertexBuffer, &stride, &offset );
    g_pImmediateContext->IASetInputLayout( qrd->inputLayout );
    g_pImmediateContext->IASetIndexBuffer( qrd->indexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    g_pImmediateContext->VSSetShader( qrd->vertexShader, nullptr, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_DrawState.viewRenderData.constantBuffer );
    g_pImmediateContext->PSSetShader( qrd->pixelShader, nullptr, 0 );
    g_pImmediateContext->PSSetSamplers( 0, 1, &image->pSampler );
    g_pImmediateContext->PSSetShaderResources( 0, 1, &image->pSRV );
    g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_DrawState.quadRenderData.constantBuffer );
    g_pImmediateContext->DrawIndexed( 6, 0, 0 );
}

static const d3dImage_t* D3DR_GetAnimatedImage( textureBundle_t *bundle, float shaderTime ) {
	int		index;

	if ( bundle->isVideoMap ) {
		ri.CIN_RunCinematic(bundle->videoMapHandle);
		ri.CIN_UploadCinematic(bundle->videoMapHandle);
		return GetImageRenderInfo( tr.scratchImage[bundle->videoMapHandle] );
	}

	if ( bundle->numImageAnimations <= 1 ) {
		return GetImageRenderInfo( bundle->image[0] );
	}

	// it is necessary to do this messy calc to make sure animations line up
	// exactly with waveforms of the same frequency
	index = myftol( shaderTime * bundle->imageAnimationSpeed * FUNCTABLE_SIZE );
	index >>= FUNCTABLE_SIZE2;

	if ( index < 0 ) {
		index = 0;	// may happen with shader time offsets
	}
	index %= bundle->numImageAnimations;

    return GetImageRenderInfo( bundle->image[index] );
}

void D3DDrv_ShadowSilhouette( const float* edges, int edgeCount )
{

}

void D3DDrv_ShadowFinish( void )
{

}

void D3DDrv_DrawSkyBox( const skyboxDrawInfo_t* skybox, const float* eye_origin, const float* colorTint )
{

}

void D3DDrv_DrawBeam( const image_t* image, const float* color, const vec3_t startPoints[], const vec3_t endPoints[], int segs )
{

}

void D3DDrv_DrawStageGeneric( const shaderCommands_t *input )
{
    // todo: set input layout

	for ( int stage = 0; stage < MAX_SHADER_STAGES; stage++ )
	{
		shaderStage_t *pStage = input->xstages[stage];

		if ( !pStage )
		{
			break;
		}

        // todo: bind color data in input->svars[stage].colors

 		//
		// do multitexture
		//
		if ( pStage->bundle[1].image[0] != 0 )
		{
            // todo: Draw MT
        }
        else
        {
            // todo: texcoords in input->svars[stage].texcoords[0]

            const d3dImage_t* texture = nullptr;
			if ( pStage->bundle[0].vertexLightmap && ( (r_vertexLight->integer && !r_uiFullScreen->integer) || vdConfig.hardwareType == GLHW_PERMEDIA2 ) && r_lightmap->integer )
			{
                texture = GetImageRenderInfo( tr.whiteImage );
			}
			else 
            {
                texture = D3DR_GetAnimatedImage( &pStage->bundle[0], input->shaderTime );
            }

			D3DDrv_SetState( pStage->stateBits );

            // todo: GLR_DrawElements( input, stage, input->numIndexes, input->indexes );
        }

		// allow skipping out to show just lightmaps during development
		if ( r_lightmap->integer && ( pStage->bundle[0].isLightmap || pStage->bundle[1].isLightmap || pStage->bundle[0].vertexLightmap ) )
		{
			break;
		}
    }
}

void D3DDrv_DrawStageVertexLitTexture( const shaderCommands_t *input )
{

}

void D3DDrv_DrawStageLightmappedMultitexture( const shaderCommands_t *input )
{

}

void D3DDrv_BeginTessellate( shaderCommands_t* input )
{
    // Do nothing.
}

template<class _Type> void UpdateTessBuffer( ID3D11Buffer* gpuBuf, const _Type* cpuBuf, size_t size )
{
	D3D11_MAPPED_SUBRESOURCE map;
	g_pImmediateContext->Map( gpuBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    memcpy( map.pData, cpuBuf, size );
    g_pImmediateContext->Unmap( gpuBuf, 0 );
}

void D3DDrv_EndTessellate( shaderCommands_t* input )
{
    // Lock down the buffers
    UpdateTessBuffer( g_DrawState.tessBufs.indexes, input->indexes, sizeof(glIndex_t) * tess.numIndexes );
    UpdateTessBuffer( g_DrawState.tessBufs.xyz, input->xyz, sizeof(vec4_t) * tess.numVertexes );

	for ( int stage = 0; stage < MAX_SHADER_STAGES; stage++ )
	{
		const stageVars_t *cpuStage = &tess.svars[stage];

		if ( !cpuStage ) {
			break;
		}

        d3dTessStageBuffers_t* gpuStage = &g_DrawState.tessBufs.stages[stage];
        UpdateTessBuffer( gpuStage->colors, cpuStage->colors, sizeof(color4ub_t) * tess.numVertexes );

        for ( int tex = 0; tex < NUM_TEXTURE_BUNDLES; ++tex )
            UpdateTessBuffer( gpuStage->texCoords[tex], cpuStage->texcoords[tex], sizeof(vec2_t) * tess.numVertexes );
    }
}

void D3DDrv_DebugDrawAxis( void )
{

}

void D3DDrv_DebugDrawTris( shaderCommands_t *input )
{

}

void D3DDrv_DebugDrawNormals( shaderCommands_t *input )
{

}

void D3DDrv_DebugDrawPolygon( int color, int numPoints, const float* points )
{

}