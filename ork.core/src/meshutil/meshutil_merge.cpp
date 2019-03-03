///////////////////////////////////////////////////////////////////////////////
// MicroOrk (Orkid)
// Copyright 1996-2013, Michael T. Mayers
// Provided under the MIT License (see LICENSE.txt)
///////////////////////////////////////////////////////////////////////////////

#include <ork/types.h>
#include <ork/plane.h>
#include <ork/meshutil/meshutil.h>
#include <ork/mutex.h>
#include <ork/thread.h>
#include <ork/timer.h>

///////////////////////////////////////////////////////////////////////////////
namespace ork { namespace MeshUtil {
///////////////////////////////////////////////////////////////////////////////

void toolmesh::MergeMaterialsFromToolMesh( const toolmesh& from )
{
	for( const auto& item : from.mShadingGroupToMaterialMap )
	{
		const std::string& key = item.first;
		const auto& val = item.second;

		const auto& itf = mShadingGroupToMaterialMap.find(key);
		OrkAssert(itf==mShadingGroupToMaterialMap.end());
		mShadingGroupToMaterialMap[key] = val;
	}
	for( auto itm : from.mMaterialsByShadingGroup )
	{	const std::string& key = itm.first;
		MaterialBase* val = itm.second;

		auto itf = mMaterialsByShadingGroup.find(key);
		OrkAssert(itf==mMaterialsByShadingGroup.end());
		mMaterialsByShadingGroup[key] = val;
	}
	for( auto itm : from.mMaterialsByName )
	{
		const std::string& key = itm.first;
		MaterialBase* val = itm.second;

		auto itf = mMaterialsByName.find(key);
		OrkAssert(itf==mMaterialsByName.end());
		mMaterialsByName[key] = val;
	}

}

///////////////////////////////////////////////////////////////////////////////

void toolmesh::MergeSubMesh( const submesh& src )
{
	MergeSubMesh( src, "default" );
}

///////////////////////////////////////////////////////////////////////////////

void toolmesh::MergeSubMesh( const toolmesh& src, const submesh* pgrp, const char* newname )
{
	Timer t;
	t.Start();

	submesh* pnewgroup = FindSubMesh( newname );
	if( 0 == pnewgroup ) 
	{
		pnewgroup = mPolyGroupLut.AddSorted(newname,new submesh)->second;
	}
	int inumpingroup = pgrp->GetNumPolys();
	for( int i=0; i<inumpingroup; i++ )
	{	const poly& ply = pgrp->RefPoly( i );
		int inumpv = ply.GetNumSides();
		poly NewPoly;
		NewPoly.miNumSides = inumpv;
		for( int iv=0; iv<inumpv; iv++ )
		{	int ivi = ply.GetVertexID(iv);
			const vertex& vtx = pgrp->RefVertexPool().GetVertex( ivi );
			int inewvi = pnewgroup->MergeVertex( vtx );
			NewPoly.miVertices[iv] = inewvi;
		}
		NewPoly.SetAnnoMap(ply.GetAnnoMap());
		pnewgroup->MergePoly(NewPoly);
	}
	float ftime = t.SecsSinceStart();
	printf( "<<PROFILE>> <<toolmesh::MergeSubMesh %f seconds>>\n", ftime );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct MergeToolMeshQueueItem
{
	const MeshUtil::submesh*	mpSourceSubMesh;
	MeshUtil::submesh*			mpDestSubMesh;
	std::string					destname;

	MergeToolMeshQueueItem() : mpSourceSubMesh(0), mpDestSubMesh(0) {}

	void DoIt(int ithread) const;
};

///////////////////////////////////////////////////////////////////////////////

struct MergeToolMeshQueue
{
	int													miNumFinished;
	int													miNumQueued;
	LockedResource< std::vector<MergeToolMeshQueueItem> >	mJobSet;
	Mutex												mSourceMutex;

	MergeToolMeshQueue() 
		: miNumFinished(0)
		, miNumQueued(0)
		, mSourceMutex()
		{}
};

void MergeToolMeshQueueItem::DoIt(int ithread) const
{
	Timer t;
	t.Start();
	int inump = mpSourceSubMesh->GetNumPolys();
	for( int i=0; i<inump; i++ )
	{	
		if( i==0x194 && inump==0x145b )
		{
			printf( "yo\n" );
		}
		const poly & ply = mpSourceSubMesh->RefPoly(i);
		int inumv = ply.GetNumSides();
		if (inumv > kmaxsidesperpoly) {
			OrkAssert( false );
			continue;
		}
		int merged[kmaxsidesperpoly];
		for(int i=0 ; i<inumv ; i++)
			merged[i] = mpDestSubMesh->MergeVertex( mpSourceSubMesh->RefVertexPool().VertexPool[ ply.miVertices[ i ] ] );
		poly polyA(merged, inumv);
		polyA.SetAnnoMap(ply.GetAnnoMap());
		mpDestSubMesh->MergePoly( polyA );
	}
	float ftime = t.SecsSinceStart();
	printf( "<<PROFILE>> <<toolmesh::MergeToolMeshThreaded  Thread<%d> Dest<%s> NumPolys<%d> %f seconds>>\n", ithread, destname.c_str(), inump, ftime );
}

struct MergeToolMeshThreadData
{
	MergeToolMeshQueue*	mQ;
	int					miThreadIndex;
};

struct MergeToolMeshJobThread : public autothread 
{
	MergeToolMeshJobThread( MergeToolMeshThreadData* thread_data ) : mData(thread_data ) {}

	MergeToolMeshThreadData* mData;

	void run() override
	{
		MergeToolMeshQueue* Q = mData->mQ;

		bool bdone = false;
		while( ! bdone )
		{
			std::vector<MergeToolMeshQueueItem>& qq = Q->mJobSet.LockForWrite();
			if( qq.size() )
			{
				std::vector<MergeToolMeshQueueItem>::iterator it = (qq.end()-1);
				MergeToolMeshQueueItem qitem = *it;
				qq.erase(it);
				Q->mJobSet.Unlock();
				////////////////////////////////
				qitem.DoIt(mData->miThreadIndex);
				////////////////////////////////
				Q->mSourceMutex.Lock();
				{
					Q->miNumFinished++;
					bdone = (Q->miNumFinished==Q->miNumQueued);
					//qitem.mpSourceToolMesh->RemoveSubMesh(qitem.mSourceSubName);
				}
				Q->mSourceMutex.Unlock();
				////////////////////////////////
			}
			else
			{
				Q->mJobSet.Unlock();
				bdone=true;
			}
		}
	}
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void toolmesh::MergeToolMeshThreadedExcluding( const toolmesh & sr, int inumthreads, const std::set<std::string>& ExcludeSet )
{
	Timer t;
	t.Start();

	MergeToolMeshQueue Q;
	std::vector<MergeToolMeshQueueItem>& QV = Q.mJobSet.LockForWrite();
	{
		for( orklut<std::string, submesh*>::const_iterator it=sr.mPolyGroupLut.begin(); it!=sr.mPolyGroupLut.end(); it++ )
		{	const submesh& src_grp = *it->second;
			const std::string& name = it->first;
			
			if( ExcludeSet.find(name)==ExcludeSet.end() )
			{
				submesh& dest_grp = MergeSubMesh( name.c_str() );
				//////////////////////////////
				MergeToolMeshQueueItem qitem;
				qitem.mpSourceSubMesh = & src_grp;
				qitem.mpDestSubMesh = & dest_grp;
				qitem.destname = name;
				//////////////////////////////
				QV.push_back(qitem);
				//////////////////////////////
			}
		}
	}
	Q.miNumQueued = (int) QV.size();
	Q.mJobSet.Unlock();
	/////////////////////////////////////////////////////////
	// start threads
	/////////////////////////////////////////////////////////
	std::vector<ork::autothread*>	ThreadVect;
	for( int ic=0; ic<inumthreads; ic++ )
	{
		MergeToolMeshThreadData* thread_data = new MergeToolMeshThreadData;
		thread_data->mQ = & Q;
		thread_data->miThreadIndex = ic;
		auto job_thread = new MergeToolMeshJobThread( thread_data );
		ThreadVect.push_back(job_thread);
	}
	/////////////////////////////////////////////////////////
	// wait for threads
	/////////////////////////////////////////////////////////
	for( auto it=ThreadVect.begin(); it!=ThreadVect.end(); it++ )
	{
		auto job = (*it);
		job->join();
        delete job;
	}
	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	MergeMaterialsFromToolMesh( sr );
	float ftime = t.SecsSinceStart();
	printf( "<<PROFILE>> <<toolmesh::MergeToolMeshThreaded %f seconds>>\n", ftime );
}

void toolmesh::MergeToolMeshThreaded( const toolmesh & sr, int inumthreads )
{
	const std::set<std::string> EmptySet;
	MergeToolMeshThreadedExcluding( sr, inumthreads, EmptySet );
}

///////////////////////////////////////////////////////////////////////////////

void toolmesh::MergeToolMeshAs( const toolmesh & sr, const char* pgroupname )
{	
	submesh& dest_group = MergeSubMesh( pgroupname );
	for( orklut<std::string,submesh*>::const_iterator itpg=sr.mPolyGroupLut.begin(); itpg!=sr.mPolyGroupLut.end(); itpg++ )
	{	const submesh& src_group = *itpg->second;
		int inump = src_group.GetNumPolys();
		for( int ip=0; ip<inump; ip++ )
		{	const poly & ply = src_group.RefPoly( ip );
			int inumv = ply.GetNumSides();
			if (inumv > kmaxsidesperpoly) {
				OrkAssert( false );
				continue;
			}
			int merged[kmaxsidesperpoly];
			for(int i=0 ; i<inumv ; i++)
				merged[i] = dest_group.MergeVertex( src_group.RefVertexPool().GetVertex( ply.miVertices[ i ] ) );
			poly npoly( merged, inumv );
			npoly.SetAnnoMap( ply.GetAnnoMap() );
			dest_group.MergePoly( npoly );
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

submesh& toolmesh::MergeSubMesh( const char* pname )
{	orklut<std::string,submesh*>::iterator itpg=mPolyGroupLut.find(pname);
	if( itpg == mPolyGroupLut.end() )
	{	itpg=mPolyGroupLut.AddSorted( pname, new submesh );
		itpg->second->name = pname;
	}
	submesh& pret = *itpg->second;
	return pret;
}

///////////////////////////////////////////////////////////////////////////////

submesh& toolmesh::MergeSubMesh( const char* pname, const submesh::AnnotationMap& merge_annos )
{	orklut<std::string,submesh*>::iterator itpg=mPolyGroupLut.find(pname);
	if( itpg == mPolyGroupLut.end() )
	{	
		submesh* nsm = new submesh;
		nsm->MergeAnnos(merge_annos,true);
		itpg=mPolyGroupLut.AddSorted( pname, nsm );
	}
	submesh& pret = *itpg->second;
	return pret;
}

///////////////////////////////////////////////////////////////////////////////

void submesh::MergeSubMesh( const submesh& inp_mesh )
{	
	Timer t;
	t.Start();
	int inumpingroup = inp_mesh.GetNumPolys();
	for( int i=0; i<inumpingroup; i++ )
	{	const poly& ply = inp_mesh.RefPoly( i );
		int inumpv = ply.GetNumSides();
		poly NewPoly;
		NewPoly.miNumSides = inumpv;
		for( int iv=0; iv<inumpv; iv++ )
		{	int ivi = ply.GetVertexID(iv);
			const vertex& vtx = inp_mesh.RefVertexPool().GetVertex( ivi );
			int inewvi = MergeVertex( vtx );
			NewPoly.miVertices[iv] = inewvi;
		}
		NewPoly.SetAnnoMap(ply.GetAnnoMap());
		MergePoly(NewPoly);
	}
	float ftime = t.SecsSinceStart();
	printf( "<<PROFILE>> <<submesh::MergeSubMesh %f seconds>>\n", ftime );
}

///////////////////////////////////////////////////////////////////////////////

void toolmesh::MergeSubMesh( const submesh& inp_mesh, const char* pasgroup )
{	submesh& sub_mesh = MergeSubMesh( pasgroup );
	sub_mesh.MergeSubMesh(inp_mesh);
}

///////////////////////////////////////////////////////////////////////////////

void submesh::MergePoly( const poly& ply )
{	int ipolyindex = GetNumPolys();
	poly nply = ply;
	int inumv = ply.GetNumSides();
	OrkAssert( inumv<=kmaxsidesperpoly );
	///////////////////////////////
	// zero area poly removal
	switch( inumv )
	{	case 3:
		{	if(		(ply.miVertices[0] == ply.miVertices[1])
				||	(ply.miVertices[1] == ply.miVertices[2])
				||	(ply.miVertices[2] == ply.miVertices[0]) )
			{
				printf( "toolmesh::MergePoly() removing zero area tri<%d %d %d>\n",
					ply.miVertices[0],
					ply.miVertices[1],
					ply.miVertices[2] );

				return;
			}

		}
		case 4:
		{	if(		(ply.miVertices[0] == ply.miVertices[1])
				||	(ply.miVertices[0] == ply.miVertices[2])
				||	(ply.miVertices[0] == ply.miVertices[3])
				||	(ply.miVertices[1] == ply.miVertices[2])
				||	(ply.miVertices[1] == ply.miVertices[3])
				||	(ply.miVertices[2] == ply.miVertices[3]) )
			{
				printf( "toolmesh::MergePoly() removing zero area quad<%d %d %d %d>\n",
					ply.miVertices[0],
					ply.miVertices[1],
					ply.miVertices[2],
					ply.miVertices[3] );

				return;
			}

		}
		// TODO n-sided polys
	}
	//////////////////////////////
	// dupe check
	U64 ucrc = ply.HashIndices();
	HashU64IntMap::iterator itfhm = mpolyhashmap.find( ucrc );
	///////////////////////////////
	if( itfhm == mpolyhashmap.end() ) // no match
	{	int inewpi = (int) mMergedPolys.size();
		mpolyhashmap[ucrc]=inewpi;
		//////////////////////////////////////////////////
		// connect to vertices
		for( int i=0; i<inumv; i++ )
		{	int iv = ply.miVertices[i];
			vertex& vtx = mvpool.GetVertex(iv);
			//vtx.ConnectToPoly(inewpi);
		}
		//////////////////////////////////////////////////
		// add edges
		if( mbMergeEdges )
		{	for( int i=0; i<inumv; i++ )
			{	int i0 = (i);
				int i1 = (i+1)%inumv;
				int iv0 = ply.GetVertexID(i0);
				int iv1 = ply.GetVertexID(i1);
				edge Edge( iv0, iv1 );
				nply.mEdges[i] = MergeEdge( Edge, ipolyindex );
			}
		}
		nply.SetAnnoMap(ply.GetAnnoMap());
		mMergedPolys.push_back( nply );
		//////////////////////////////////////////////////
		// add n sided counters
		mPolyTypeCounter[ inumv ]++;
		//////////////////////////////////////////////////
		float farea = ply.ComputeArea( mvpool, ork::CMatrix4::Identity );
		mfSurfaceArea += farea;
	}
	mAABoxDirty = true;
}

///////////////////////////////////////////////////////////////////////////////

int vertexpool::MergeVertex( const vertex & vtx, int inidx ) 
{	int ioutidx = -1;
	U64 vhash = vtx.Hash();
	HashU64IntMap::const_iterator it = VertexPoolMap.find( vhash );
	if( VertexPoolMap.end() != it )
	{	int iother = it->second;
		const vertex & OtherVertex = VertexPool[ iother ];
		//boost::Crc64 otherCRC = boost::crc64( (const void *) & vtx, sizeof( vertex ) );
		//U32 otherCRC = Crc32( (const unsigned char *) & OtherVertex, sizeof( vertex ) );
		//OrkAssert( CCRC::DoesDataMatch( & vtx, & OtherVertex, sizeof( vertex ) ) );
		ioutidx = iother;
	}
	else
	{	int ipv = (int) VertexPool.size();
		VertexPool.push_back( vtx );
		VertexPoolMap[vhash]=ipv;
		ioutidx = ipv;
	}
	return ioutidx;
}

///////////////////////////////////////////////////////////////////////////////

U64 submesh::MergeEdge( const edge & ed, int ipolyindex )
{	U64 crcA = ed.GetHashKey();
	HashU64IntMap::const_iterator itfind = mEdgeMap.find( crcA );
	
	int ieee = -1;

	if( mEdgeMap.end() != itfind )
	{	
		ieee = itfind->second;
		edge & other = mEdges[ ieee ];
		U64 crcB = other.GetHashKey();
		OrkAssert( ed.Matches( other) );
	}
	else
	{	ieee = (int) mEdges.size();
		mEdges.push_back( ed );
		mEdgeMap[crcA]=ieee;
	}
	if( ipolyindex >= 0 )
	{	mEdges[ ieee ].ConnectToPoly( ipolyindex );
	}
	
	mAABoxDirty = true;
	return crcA;
}

///////////////////////////////////////////////////////////////////////////////
}}
///////////////////////////////////////////////////////////////////////////////
