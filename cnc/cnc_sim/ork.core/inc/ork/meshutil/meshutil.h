///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////

#include <unordered_map>
#include <map>
#include <set>
#include <ork/crc.h>
#include <ork/crc64.h>
//#include <orktool/filter/filter.h>
#include <ork/cvector3.h>
#include <ork/cvector4.h>
#include <ork/box.h>
#include <ork/fixedstring.h>
#include <algorithm>
#include <ork/lut.h>
#include <ork/path.h>

struct DaeReadOpts;
struct DaeWriteOpts;

///////////////////////////////////////////////////////////////////////////////

namespace ork { namespace MeshUtil {

typedef fxstring64 keystring_t;

struct MaterialBase
{

};

struct MaterialBindingItem
{
	std::string mMaterialName;
	std::string mMaterialDaeId;
	//std::vector<FCDMaterialInstanceBind*> mBindings;
};

typedef std::map<std::string,MaterialBindingItem> material_semanticmap_t;

class Light //: public ork::Object
{
	//RttiDeclareAbstract(Light, ork::Object);

public:

	std::string		mName;
	CMatrix4		mWorldMatrix;
	CVector3		mColor;
	float			mIntensity;
	float			mShadowSamples;
	float			mShadowBias;
	float			mShadowBlur;
	bool			mbSpecular;
	bool			mbIsShadowCaster;

	virtual bool AffectsSphere( const CVector3& center, float radius ) const { return false; }
	virtual bool AffectsAABox( const AABox& aab ) const { return false; }

	Light() 
		: mColor(1.0f,1.0f,1.0f)
		, mIntensity(1.0f) 
		, mbSpecular(false)
		, mShadowSamples(1.0f)
		, mShadowBlur(0.0f)
		, mShadowBias(0.2f)
		, mbIsShadowCaster(false)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

class LightContainer //: public ork::Object
{
	//RttiDeclareConcrete(LightContainer, ork::Object);
public:

	orklut<keystring_t,Light*>	mLights;
};

///////////////////////////////////////////////////////////////////////////////

struct HashU6432
    : public std::unary_function<U64, std::size_t>
{
    std::size_t operator()(U64 v) const
    {
		U64 sh = v>>32;
		size_t h = size_t(sh);
		return h;
    }
	bool operator() (U64 s1, U64 s2) const
	{
		return s1 < s2;
	}
};
	
struct Hash3232
    : public std::unary_function<int, std::size_t>
{
    std::size_t operator()(int v) const
    {
		size_t h = size_t(v);
		return h;
    }
	bool operator() (int s1, int s2) const
	{
		return s1 < s2;
	}
};
///////////////////////////////////////////////////////////////////////////////

typedef std::unordered_map<U64,int,HashU6432>	HashU64IntMap;
typedef std::unordered_map<int,int,Hash3232>	HashIntIntMap;

static const int kmaxpolysperedge = 4;

class edge
{
	int	miVertexA;
	int	miVertexB;
	int miNumConnectedPolys;
	int miConnectedPolys[kmaxpolysperedge];

public:

	int GetVertexID( int iv ) const 
	{
		int id = -1;

		switch( iv )
		{
			case 0: id = miVertexA; break;
			case 1: id = miVertexB; break;
			default:
				OrkAssert( false ); 
				break;
		}

		return id;
	}

	U64 GetHashKey( void ) const;
	bool Matches( const edge & other ) const;

	edge() 
		: miVertexA(-1)
		, miVertexB(-1) 
		, miNumConnectedPolys( 0 )
	{
		for( int i=0; i<kmaxpolysperedge; i++ )  miConnectedPolys[i]=-1;
	}

	edge( int iva, int ivb )
		: miNumConnectedPolys( 0 )
		, miVertexA(iva)
		, miVertexB(ivb)
	{
		for( int i=0; i<kmaxpolysperedge; i++ )  miConnectedPolys[i]=-1;
	}

	void ConnectToPoly( int ipoly );

	int GetNumConnectedPolys( void ) const { return miNumConnectedPolys; }

	int GetConnectedPoly( int ip ) const 
	{
		OrkAssert( ip<miNumConnectedPolys );
		return miConnectedPolys[ ip ];
	}

};

///////////////////////////////////////////////////////////////////////////////

struct uvmapcoord
{
	CVector3 mMapBiNormal;
	CVector3 mMapTangent;
	CVector2 mMapTexCoord;

	void Lerp( const uvmapcoord & ina, const uvmapcoord &inb, float flerp );

	uvmapcoord operator+ ( const uvmapcoord & ina ) const;
	uvmapcoord operator* ( const float Scalar ) const;

	uvmapcoord()
	{
	}

	void Clear( void )
	{
		mMapBiNormal = CVector3();
		mMapTangent = CVector3();
		mMapTexCoord = CVector2();
	}
};

///////////////////////////////////////////////////////////////////////////////

struct vertex
{
	static const int kmaxinfluences = 4;
	static const int kmaxcolors = 2;
	static const int kmaxuvs = 2;
	static const int kmaxconpoly = 8;

	CVector3	mPos;
	CVector3	mNrm;

	int				miNumWeights;
	int				miNumColors;
	int				miNumUvs;

	std::string	mJointNames[kmaxinfluences];

	CVector4	mCol[kmaxcolors];
	uvmapcoord	mUV[kmaxuvs];
	float		mJointWeights[kmaxinfluences];

	vertex()
		: miNumWeights( 0 )
		, miNumColors( 0 )
		, miNumUvs( 0 )
	{
		for( int i=0; i<kmaxcolors; i++ )
		{
			mCol[i] = CVector4::White();
		}
		for( int i=0; i<kmaxinfluences; i++ )
		{
			mJointNames[i] = "";
			mJointWeights[i] = float(0.0f);
		}
	}

	vertex Lerp( const vertex & vtx, float flerp ) const;
	void Lerp( const vertex& a, const vertex & b, float flerp );

	const CVector3& Pos() const { return mPos; }

	void Center( const vertex** pverts, int icnt );

	U64 Hash() const;

};

///////////////////////////////////////////////////////////////////////////////

struct vertexpool
{
	static const vertexpool	EmptyPool;

	HashU64IntMap			VertexPoolMap;
	std::vector<vertex>		VertexPool;

	int MergeVertex( const vertex & vtx, int idx=-1 );

	const vertex & GetVertex( int ivid ) const
	{
		OrkAssert( std::vector<vertex>::size_type(ivid)<VertexPool.size() );
		return VertexPool[ ivid ];
	}
	vertex & GetVertex( int ivid )
	{
		OrkAssert( std::vector<vertex>::size_type(ivid)<VertexPool.size() );
		return VertexPool[ ivid ];
	}

	size_t GetNumVertices( void ) const
	{
		return VertexPool.size();
	}

	vertexpool();
};

///////////////////////////////////////////////////////////////////////////////

struct AnnoMap
{
	std::map<std::string,std::string>	mAnnotations;
	AnnoMap* Fork() const;
	static std::set<AnnoMap*>	gAllAnnoSets;
	void SetAnnotation( const std::string& key, const std::string& val );
	const std::string& GetAnnotation( const std::string& annoname ) const;

	AnnoMap();
	~AnnoMap();
};

///////////////////////////////////////////////////////////////////////////////

static const int kmaxsidesperpoly = 5;

class poly
{
	const AnnoMap*	mAnnotationSet;

public:

	static const U64 Inv = 0xffffffffffffffffL;
	const AnnoMap* GetAnnoMap() const { return mAnnotationSet; }
	void SetAnnoMap( const AnnoMap* pmap ) { mAnnotationSet=pmap; }

	const std::string& GetAnnotation( const std::string& annoname ) const;

	int				miVertices[kmaxsidesperpoly];
	U64				mEdges[kmaxsidesperpoly];
	int				miNumSides;

	int GetNumSides( void ) const { return miNumSides; }

	int GetVertexID( int i ) const
	{
		OrkAssert( i<miNumSides );
		return miVertices[ i ];
	}

	poly()
		: miNumSides( 0 )
		, mAnnotationSet(0)
	{
		for( int i=0; i<kmaxsidesperpoly; i++ )
		{
			miVertices[i] = -1;
			mEdges[i] = Inv;
		}
	}

	poly( int ia, int ib, int ic )
		: miNumSides( 3 )
		, mAnnotationSet(0)
	{
		miVertices[0] = ia;
		miVertices[1] = ib;
		miVertices[2] = ic;
		mEdges[0] = Inv;
		mEdges[1] = Inv;
		mEdges[2] = Inv;
		for(int i=3 ; i<kmaxsidesperpoly ; i++) {
			miVertices[i] = -1;
			mEdges[i] = Inv;
		}
	}

	poly( int ia, int ib, int ic, int id )
		: miNumSides( 4 )
		, mAnnotationSet(0)
	{
		miVertices[0] = ia;
		miVertices[1] = ib;
		miVertices[2] = ic;
		miVertices[3] = id;
		mEdges[0] = Inv;
		mEdges[1] = Inv;
		mEdges[2] = Inv;
		mEdges[3] = Inv;
		for(int i=4 ; i<kmaxsidesperpoly ; i++) {
			miVertices[i] = -1;
			mEdges[i] = Inv;
		}
	}

	poly( const int verts[], int numSides )
		: miNumSides(numSides)
		, mAnnotationSet(0)
	{
		for(int i=0 ; i<numSides ; i++) {
			miVertices[i] = verts[i];
			mEdges[i] = Inv;
		}
		for(int i=numSides ; i<kmaxsidesperpoly ; i++) {
			miVertices[i] = -1;
			mEdges[i] = Inv;
		}
	}

	// vertex clockwise around the poly from the given one
	int VertexCW(int vert) const;
	// vertex counter-clockwise around the poly from the given one
	int VertexCCW(int vert) const;

	vertex ComputeCenter( const vertexpool &vpool ) const;
	float ComputeEdgeLength( const vertexpool &vpool, const CMatrix4 & MatRange, int iedge ) const;
	float ComputeArea( const vertexpool &vpool, const CMatrix4 & MatRange ) const;
	CVector3 ComputeNormal( const vertexpool& vpool) const;

	U64 HashIndices( void ) const;

};

///////////////////////////////////////////////////////////////////////////////

struct IndexTestContext
{
	int					iset;
	int					itest;
	std::set<int>			PairedIndices[2];
	std::set<int>			PairedIndicesCombined;
	std::set<int>			CornerIndices;
};

///////////////////////////////////////////////////////////////////////////////

class toolmesh;
struct submesh;

struct annopolylut
{
	std::map<U64,const AnnoMap*>	mAnnoMap;
	virtual U64 HashItem( const submesh& tmesh, const poly& item ) const = 0;
	const AnnoMap* Find( const submesh& tmesh, const poly& item ) const;
};
struct annopolyposlut : public annopolylut
{
	virtual U64 HashItem( const submesh& tmesh, const poly& item ) const;
};


///////////////////////////////////////////////////////////////////////////////

struct submesh
{
	typedef std::map<std::string,std::string> AnnotationMap;

	std::string						name;
	AnnotationMap					mAnnotations;
	float							mfSurfaceArea;
	vertexpool						mvpool;
	HashU64IntMap					mpolyhashmap;
	std::vector<edge>					mEdges;
	HashU64IntMap					mEdgeMap;
	std::vector<poly>					mMergedPolys;
	int								mPolyTypeCounter[kmaxsidesperpoly];
	bool							mbMergeEdges;

	/////////////////////////////////////
	// these are mutable so we can get bounding boxes faster with const refs to toolmesh's
	mutable AABox					mAABox;
	mutable bool					mAABoxDirty;
	/////////////////////////////////////

	void SplitOnAnno( toolmesh& out, const std::string& annokey ) const;
	void SplitOnAnno( toolmesh& out, const std::string& prefix, const std::string& annokey ) const;
	void SplitOnAnno( toolmesh& out, const std::string& annokey, const AnnotationMap& MergeAnnos ) const;
	void ImportPolyAnnotations( const annopolylut& apl );
	void ExportPolyAnnotations( annopolylut& apl ) const;

	void SetAnnotation( const char* annokey, const char* annoval );
	const char* GetAnnotation( const char* annokey ) const;
	AnnotationMap& RefAnnotations() { return mAnnotations; }
	const AnnotationMap& RefAnnotations() const { return mAnnotations; }

	void MergeAnnos( const AnnotationMap& mrgannos, bool boverwrite );

	//////////////////////////////////////////////////////////////////////////////

	const vertexpool&		RefVertexPool() const { return mvpool; }
	const edge&				RefEdge( U64 edgekey ) const;
	poly&					RefPoly( int i );
	const poly&				RefPoly( int i ) const;
	const std::vector<poly>&	RefPolys() const;

	//////////////////////////////////////////////////////////////////////////////

	int MergeVertex( const vertex& vtx, int idx=-1 );
	U64 MergeEdge( const edge& ed, int ipolyindex=-1 );
	void MergePoly( const poly& ply );
	void MergeSubMesh( const submesh& oth );

	//////////////////////////////////////////////////////////////////////////////

	int	GetNumPolys( int inumsides=0 ) const;
	void FindNSidedPolys( std::vector<int>& output, int inumsides ) const;
	void GetConnectedPolys( const edge& ed, std::set<int>& output ) const;
	void GetEdges( const poly& ply, std::vector<edge>& Edges ) const;
	void GetAdjacentPolys( int ply, std::set<int>& output ) const;
	const U64 GetEdgeBetween( int a, int b ) const;

	const AABox& GetAABox() const;

	/////////////////////////////////////////////////////////////////////////

	void SetVertexPool( const vertexpool & vpool ) { mvpool = vpool; }

	/////////////////////////////////////////////////////////////////////////

	void Triangulate( submesh *poutsmesh ) const;
	void TrianglesToQuads( submesh *poutsmesh ) const;
	void SubDivQuads( submesh *poutsmesh ) const;
	void SubDivTriangles( submesh *poutsmesh ) const;
	void SubDiv( submesh *poutsmesh ) const;

	/////////////////////////////////////////////////////////////////////////

	submesh(const vertexpool& vpool = vertexpool::EmptyPool);
	~submesh();
	
};

///////////////////////////////////////////////////////////////////////////////

class toolmesh
{
	std::map<std::string,MaterialBase*>	mMaterialsByShadingGroup;
	std::map<std::string,MaterialBase*>	mMaterialsByName;

	CVector4							mRangeScale;
	CVector4							mRangeTranslate;
	CMatrix4							mMatRange;
	std::map<std::string,std::string>		mAnnotations;
	orklut<std::string, submesh*>		mPolyGroupLut;
	material_semanticmap_t				mShadingGroupToMaterialMap;
	LightContainer						mLights;
	bool								mbMergeEdges;
	//ork::lev2::MaterialMap				mFxmMaterialMap;

public:

	void SetMergeEdges( bool bflg ) { mbMergeEdges=bflg; }

	/////////////////////////////////////////////////////////////////////////
	
	void Dump(const std::string& comment) const;

	/////////////////////////////////////////////////////////////////////////

	//const ork::lev2::MaterialMap& RefFxmMaterialMap() const { return mFxmMaterialMap; }
	const std::map<std::string,MaterialBase*>& RefMaterialsBySG() const { return mMaterialsByShadingGroup; }
	const std::map<std::string,MaterialBase*>& RefMaterialsByName() const { return mMaterialsByName; }
	const LightContainer& RefLightContainer() const { return mLights; }
	LightContainer& RefLightContainer() { return mLights; }
	
	void CopyMaterialsFromToolMesh( const toolmesh& from );
	void MergeMaterialsFromToolMesh( const toolmesh& from );

	void RemoveSubMesh( const std::string& pgroup );
	void Prune();

	/////////////////////////////////////////////////////////////////////////

	void SetAnnotation( const char* annokey, const char* annoval );
	const char* GetAnnotation( const char* annokey ) const;

	/////////////////////////////////////////////////////////////////////////
	void WriteToWavefrontObj( const Path& outpath ) const;
	void ReadFromWavefrontObj( const Path& inpath );
	/////////////////////////////////////////////////////////////////////////
	//void WriteToDaeFile( const Path& outpath, const DaeWriteOpts& writeopts ) const;
	//void ReadFromDaeFile( const Path& inpath, DaeReadOpts& readopts );
	/////////////////////////////////////////////////////////////////////////
	//void ReadFromXGM( const Path& inpath );
	//void WriteToXgmFile( const Path& outpath ) const;
	//void WriteToRgmFile( const Path& outpath ) const;
	/////////////////////////////////////////////////////////////////////////
	//void ReadAuto( const Path& outpath );
	//void WriteAuto( const Path& outpath ) const;
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////

	AABox GetAABox() const;

	/////////////////////////////////////////////////////////////////////////

	void SetRangeTransform( const CVector4 &VScale, const CVector4 & VTrans );

	/////////////////////////////////////////////////////////////////////////
	
	void MergeToolMeshAs( const toolmesh & sr, const char* pgroupname );
	void MergeToolMeshThreadedExcluding( const toolmesh & sr, int inumthreads, const std::set<std::string>& ExcludeSet );
	void MergeToolMeshThreaded( const toolmesh & sr, int inumthreads );
	void MergeSubMesh( const toolmesh& src, const submesh* pgrp, const char* newname );
	void MergeSubMesh( const submesh& pgrp, const char* newname );
	void MergeSubMesh( const submesh& pgrp );
	submesh& MergeSubMesh( const char* pname );
	submesh& MergeSubMesh( const char* pname, const submesh::AnnotationMap& merge_annos );

	/////////////////////////////////////////////////////////////////////////

	const orklut<std::string, submesh*>& RefSubMeshLut() const;
	const material_semanticmap_t& RefShadingGroupToMaterialMap() const { return mShadingGroupToMaterialMap; }
	material_semanticmap_t& RefShadingGroupToMaterialMap() { return mShadingGroupToMaterialMap; }
	
	int GetNumSubMeshes() const { return int(mPolyGroupLut.size()); }

	const submesh* FindSubMeshFromMaterialName(const std::string& materialname ) const;
	submesh* FindSubMeshFromMaterialName(const std::string& materialname );
	const submesh* FindSubMesh(const std::string& grpname ) const;
	submesh* FindSubMesh(const std::string& grpname );

	/////////////////////////////////////////////////////////////////////////

	toolmesh();
	~toolmesh();

private:

	toolmesh(const toolmesh&oth) 
	{
		OrkAssert(false);
	}

};

///////////////////////////////////////////////////////////////////////////////
/*
struct TriStripperPrimGroup
{
	std::vector<unsigned int> mIndices;
};

class TriStripper
{
	triangle_stripper::tri_stripper tristripper;
	std::vector<TriStripperPrimGroup> mStripGroups;
	TriStripperPrimGroup			mTriGroup;

public:

	TriStripper( const std::vector<unsigned int> &InTriIndices, int icachesize, int iminstripsize );

	const std::vector<TriStripperPrimGroup> & GetStripGroups( void ) const
	{
		return mStripGroups;
	}

	const std::vector<unsigned int> & GetStripIndices( int igroup ) const
	{
		return mStripGroups[igroup].mIndices;
	}

	const std::vector<unsigned int> & GetTriIndices( void ) const { return mTriGroup.mIndices; }

};
*/

///////////////////////////////////////////////////////////////////////////////

//class OBJ_OBJ_Filter //: public ork::tool::CAssetFilterBase
//{
	//RttiDeclareConcrete(OBJ_OBJ_Filter,ork::tool::CAssetFilterBase);
//public: //
//	OBJ_OBJ_Filter(  );
//	bool ConvertAsset( const tokenlist& toklist );
//};
/*class D3DX_OBJ_Filter : public ork::tool::CAssetFilterBase
{
	RttiDeclareConcrete(D3DX_OBJ_Filter,ork::tool::CAssetFilterBase);
public: //
	D3DX_OBJ_Filter(  );
	virtual bool ConvertAsset( const tokenlist& toklist );
};
class XGM_OBJ_Filter : public ork::tool::CAssetFilterBase
{
	RttiDeclareConcrete(XGM_OBJ_Filter,ork::tool::CAssetFilterBase);
public: //
	XGM_OBJ_Filter(  );
	virtual bool ConvertAsset( const tokenlist& toklist );
};
class OBJ_XGM_Filter : public ork::tool::CAssetFilterBase
{
	RttiDeclareConcrete(OBJ_XGM_Filter,ork::tool::CAssetFilterBase);
public: //
	OBJ_XGM_Filter(  );
	virtual bool ConvertAsset( const tokenlist& toklist );
};*/

///////////////////////////////////////////////////////////////////////////////

class AmbientLight : public Light
{
	//RttiDeclareConcrete(AmbientLight, Light);
public:
	AmbientLight(){}
	virtual bool AffectsSphere( const CVector3& center, float radius ) const { return true; }
	virtual bool AffectsAABox( const AABox& aab ) const { return true; }
};

///////////////////////////////////////////////////////////////////////////////

struct DirLight : public Light
{
	//RttiDeclareConcrete(DirLight, Light);
public:
	CVector3	mFrom;
	CVector3	mTo;

	DirLight() {}

	virtual bool AffectsSphere( const CVector3& center, float radius ) const { return true; }
	virtual bool AffectsAABox( const AABox& aab ) const { return true; }

};

///////////////////////////////////////////////////////////////////////////////

class PointLight : public Light
{
	//RttiDeclareConcrete(PointLight, Light);
public:
	CVector3	mPoint;
	float		mFalloff;
	float		mRadius;

	PointLight() : mFalloff(1.0f), mRadius(0.0f) {}

	virtual bool AffectsSphere( const CVector3& center, float radius ) const;
	virtual bool AffectsAABox( const AABox& aab ) const;
};

///////////////////////////////////////////////////////////////////////////////

/*struct FlatSubMesh
{
	lev2::EVtxStreamFormat				evtxformat;
	std::vector<int>						TrianglePolyIndices;
	std::vector<int>						QuadPolyIndices;
	std::vector<int>						MergeTriIndices;
	std::vector<lev2::SVtxV12N12B12T16>	MergeVertsT16;
	std::vector<lev2::SVtxV12N12B12T8C4>	MergeVertsT8;

	int inumverts;
	int ivtxsize;
	void* poutvtxdata;

	FlatSubMesh( const submesh& mesh );

};*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
#if defined(_USE_D3DX)
class XGM_D3DX_Filter : public ork::tool::CAssetFilterBase
{
	RttiDeclareConcrete(XGM_D3DX_Filter,ork::tool::CAssetFilterBase);
public: //
	XGM_D3DX_Filter(  );
	virtual bool ConvertAsset( const tokenlist& toklist );
};
class OBJ_D3DX_Filter : public ork::tool::CAssetFilterBase
{
	RttiDeclareConcrete(OBJ_D3DX_Filter,ork::tool::CAssetFilterBase);
public: //
	OBJ_D3DX_Filter(  );
	virtual bool ConvertAsset( const tokenlist& toklist );
};
struct UvAtlasContext
{
	const submesh&						mInpMesh;
	submesh&							mOutMesh;
	float								mfGutter;
	float								mfStretching;
	int									miTexRes;
	bool								mbDoIMT;
	float								mfAreaUnification;
	ork::file::Path						mIMTTexturePath;
	std::string							mGroupName;

	UvAtlasContext( const submesh& inmesh, submesh& outmesh );
};


bool UvAtlasSubMesh( const UvAtlasContext& Ctx );
bool UvAtlasSubMesh2( const UvAtlasContext& Ctx );
void GenerateUVAtlas( const tokenlist& options );

#endif
*/
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

} } // namespace MeshUtil
