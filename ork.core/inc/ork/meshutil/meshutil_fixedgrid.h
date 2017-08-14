///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

namespace ork { namespace MeshUtil {
///////////////////////////////////////////////////////////////////////////////
	
struct GridNode
{
	//toolmesh NodeMesh;
	std::string GridNodeName;
};

struct GridAddr
{
	int ix, iy, iz;
	GridAddr() : ix(0), iy(0), iz(0) {}
};

struct mupoly_clip_adapter
{
	typedef vertex VertexType;

	orkvector<VertexType>	mVerts;

	mupoly_clip_adapter()
	{
	}

	void AddVertex( const VertexType& v )
	{
		mVerts.push_back(v);
	}
	const VertexType& GetVertex( int idx ) const
	{
		return mVerts[idx];
	}
	
	size_t GetNumVertices() const { return mVerts.size(); }

};

class GridGraph
{
public:

	////////////////////////////////////////////////////

	int	miDimX;
	int	miDimY;
	int	miDimZ;
	int	miNumGrids;
	int	miNumFilledGrids;

	GridNode**	mppGrids;

	CVector3	vsize;
	CVector3	vmin;
	CVector3	vmax;
	CVector3	vctr;
	AABox		maab;
	float		areamax;
	float		areamin;
	float		areaavg;
	float		areatot;
	int			totpolys;
	CMatrix4	mMtxWorldToGrid;
	const int   kfixedgridsize;

	////////////////////////////////////////////////////

	GridGraph( const int fgsize = 256 );
	~GridGraph();
	void BeginPreMerge();
	void PreMergeMesh( const submesh& MeshIn );
	GridNode* GetGridNode( const GridAddr& addr );
	void SetGridNode( const GridAddr& addr, GridNode*node );
	GridAddr GetGridAddress( const CVector3& v );
	void GetCuttingPlanes(	const GridAddr& addr, 
							CPlane& topplane, CPlane& botplane,
							CPlane& lftplane, CPlane& rgtplane,
							CPlane& frnplane, CPlane& bakplane );
	
	void EndPreMerge();
	void MergeMesh( const submesh& MeshIn, toolmesh& MeshOut );

	////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////

}}
