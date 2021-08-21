#include "DLL_MAIN.h"

enum Bone : int
{
	BONE_PELVIS = 0,
	LEAN_ROOT,
	CAM_DRIVER,
	BONE_HIP,
	BONE_LOWER_SPINAL_COLUMN,
	BONE_MIDDLE_SPINAL_COLUMN,
	BONE_UPPER_SPINAL_COLUMN,
	BONE_NECK,
	BONE_HEAD,
};

enum Hitboxes : int
{
	HITBOX_HEAD = 0,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_SPINE,
	HITBOX_LEGS,
	HITBOX_ARMS,
};


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IMaterial;
struct vcollide_t;
struct model_t;
class Vector;
class CGameTrace;
struct cplane_t;
typedef CGameTrace trace_t;
struct studiohdr_t;
struct virtualmodel_t;
typedef unsigned char BYTE;
struct virtualterrainparams_t;
class CPhysCollide;
class CUtlBuffer;
class IClientRenderable;
class Quaternion;
struct mstudioanimdesc_t;
struct mstudioseqdesc_t;
struct mstudiobodyparts_t;
struct mstudiotexture_t;

class RadianEuler
{
public:
	inline RadianEuler(void) { }
	inline RadianEuler(float X, float Y, float Z) { x = X; y = Y; z = Z; }
	inline RadianEuler(Quaternion const &q);	// evil auto type promotion!!!
	inline RadianEuler(Vector const &angles);	// evil auto type promotion!!!

												// Initialization
	inline void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f) { x = ix; y = iy; z = iz; }

	//	conversion to Vector
	Vector ToQAngle(void) const;
	bool IsValid() const;
	void Invalidate();

	inline float *Base() { return &x; }
	inline const float *Base() const { return &x; }

	// array access...
	float operator[](int i) const;
	float& operator[](int i);

	float x, y, z;
};

class Quaternion				// same data-layout as engine's vec4_t,
{								//		which is a float[4]
public:
	inline Quaternion(void) {}
	inline Quaternion(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}

	inline void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f, float iw = 0.0f) { x = ix; y = iy; z = iz; w = iw; }

	float* Base() { return (float*)this; }
	const float* Base() const { return (float*)this; }

	float x, y, z, w;
};

#define DECL_ALIGN(x)			__declspec( align( x ) )
#define ALIGN16 DECL_ALIGN(16)

class ALIGN16 QuaternionAligned : public Quaternion
{
public:
	inline QuaternionAligned(void) {};
	inline QuaternionAligned(float X, float Y, float Z, float W)
	{
		Init(X, Y, Z, W);
	}
public:
	explicit QuaternionAligned(const Quaternion &vOther)
	{
		Init(vOther.x, vOther.y, vOther.z, vOther.w);
	}

	QuaternionAligned& operator=(const Quaternion &vOther)
	{
		Init(vOther.x, vOther.y, vOther.z, vOther.w);
		return *this;
	}
};

struct mstudiohitboxset_t
{
	int sznameindex;
	int numhitboxes;
	int hitboxindex;

	inline mstudiobbox_t* pHitbox(int i) const
	{
		return (mstudiobbox_t*)(((BYTE*)this) + hitboxindex) + i;
	}
};

struct mstudiobone_t
{
	int					sznameindex;
	inline char * const pszName(void) const { return ((char *)this) + sznameindex; }
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none

											// default values
	Vector				pos;
	Quaternion			quat;
	RadianEuler			rot;
	// compression scale
	Vector				posscale;
	Vector				rotscale;

	matrix				poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	inline void *pProcedure() const { if (procindex == 0) return NULL; else return  (void *)(((BYTE *)this) + procindex); };
	int					surfacepropidx;	// index into string tablefor property name
	inline char * const pszSurfaceProp(void) const { return ((char *)this) + surfacepropidx; }
	inline int			GetSurfaceProp(void) const { return surfacepropLookup; }

	int					contents;		// See BSPFlags.h for the contents flags
	int					surfacepropLookup;	// this index must be cached by the loader, not saved in the file
	int					unused[7];		// remove as appropriate
};

struct mstudiobonecontroller_t
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// BYTE index value at rest
	int					inputfield;	// 0-3 user set controller, 4 mouth
	int					unused[8];
};

struct mstudioattachment_t
{
	int					sznameindex;
	inline char * const pszName(void) const { return ((char *)this) + sznameindex; }
	unsigned int		flags;
	int					localbone;
	matrix			local; // attachment point
	int					unused[8];
};

struct studiohdr_t
{
	int    id;					// 0x0000
	int    version;				// 0x0004
	int    checksum;				// 0x0008
	char   name[64];			// 0x000C
	int    length;				// 0x004C
	Vector eye_pos;				// 0x0050
	Vector illum_pos;				// 0x005C
	Vector hull_mins;				// 0x0068
	Vector hull_maxs;             // 0x0074
	Vector view_mins;             // 0x0080
	Vector view_maxs;             // 0x008C
	int    flags;					// 0x0098
	int    num_bones;				// 0x009C
	int    bone_id;				// 0x00A0
	int    num_controllers;		// 0x00A4
	int    controller_id;			// 0x00A8
	int    num_sets;				// 0x00AC
	int    set_id;				// 0x00B0

	__forceinline mstudiobone_t* GetBone(int index) const {
		return (mstudiobone_t*)(((BYTE*)this) + bone_id) + index;
	}

	__forceinline mstudiohitboxset_t* pHitboxSet(int index) const {
		return (mstudiohitboxset_t*)(((BYTE*)this) + set_id) + index;
	}

	inline mstudiobbox_t *pHitbox(int i, int set) const
	{
		mstudiohitboxset_t const *s = pHitboxSet(set);
		if (!s)
			return NULL;

		return s->pHitbox(i);
	};
};

class virtualgroup_t
{
public:
	void *cache;
	const studiohdr_t *GetStudioHdr(void) const
	{
		return (studiohdr_t *)cache;
	}

	CUtlVector< int > boneMap;				// maps global bone to local bone
	CUtlVector< int > masterBone;			// maps local bone to global bone
	CUtlVector< int > masterSeq;			// maps local sequence to master sequence
	CUtlVector< int > masterAnim;			// maps local animation to master animation
	CUtlVector< int > masterAttachment;	// maps local attachment to global
	CUtlVector< int > masterPose;			// maps local pose parameter to global
	CUtlVector< int > masterNode;			// maps local transition nodes to global
};

struct virtualsequence_t
{
	int	flags;
	int activity;
	int group;
	int index;
};

struct virtualgeneric_t
{
	int group;
	int index;
};

struct virtualmodel_t
{
	char pad_mutex[0x8];
	CUtlVector< virtualsequence_t > m_seq;
	CUtlVector< virtualgeneric_t > m_anim;
	CUtlVector< virtualgeneric_t > m_attachment;
	CUtlVector< virtualgeneric_t > m_pose;
	CUtlVector< virtualgroup_t > m_group;
	CUtlVector< virtualgeneric_t > m_node;
	CUtlVector< virtualgeneric_t > m_iklock;
	CUtlVector< unsigned short > m_autoplaySequences;
};

class CStudioHdr
{
public:
	mutable studiohdr_t* m_pStudioHdr; //0x0000 
	mutable virtualmodel_t* m_pVModel; //0x0004 
	char				pad_unknown[0x4]; //0x0008 
	mutable CUtlVector< const studiohdr_t* > m_pStudioHdrCache; //0x000C 
	mutable int			m_nFrameUnlockCounter; //0x0020 
	int*				m_pFrameUnlockCounter; //0x0024 
	char				pad_mutex[0x8]; //0x0028 
	CUtlVector< int >	m_boneFlags; //0x0030
	CUtlVector< int >	m_boneParent; //0x0050 
	inline int			numbones(void) const { return m_pStudioHdr->num_bones; };
	inline mstudiobone_t *pBone(int i) const { return m_pStudioHdr->GetBone(i); };
	inline virtualmodel_t		*GetVirtualModel(void) const { return m_pVModel; };
	const studiohdr_t *GroupStudioHdr(int i)
	{
		const studiohdr_t *pStudioHdr = m_pStudioHdrCache[i];

		if (!pStudioHdr)
		{
			virtualgroup_t *pGroup = &m_pVModel->m_group[i];
			pStudioHdr = pGroup->GetStudioHdr();
		}

		//Assert(pStudioHdr);
		return pStudioHdr;
	}
};

struct studiohwdata_t
{
	int					m_RootLOD;	// calced and clamped, nonzero for lod culling
	int					m_NumLODs;
	void* m_pLODs;
	int					m_NumStudioMeshes;

	inline float LODMetric(float unitSphereSize) const { return (unitSphereSize != 0.0f) ? (100.0f / unitSphereSize) : 0.0f; }
	inline int GetLODForMetric(float lodMetric) const
	{
		return 0;
	}
};

struct vertexFileHeader_t;

class IVModelInfo
{
public:
	inline void* GetModel(int Index) {
		return getvfunc<void*(__thiscall *)(void*, int)>(this, 1)(this, Index);
	}
	int	GetModelIndex(const char *name)
	{
		typedef int(__thiscall* oGetModelName)(PVOID, const char *);
		return getvfunc< oGetModelName >(this, 2)(this, name);
	}
	const char *GetModelName(const model_t *mod)
	{
		typedef const char *(__thiscall* oGetModelName)(PVOID, const model_t*);
		return getvfunc< oGetModelName >(this, 3)(this, mod);
	}
	inline const char* GetModelName(const void* Model) {
		return getvfunc<const char*(__thiscall *)(void*, const void*)>(this, 3)(this, Model);
	}
	studiohdr_t	*GetStudioModel(const model_t *mod)
	{
		typedef studiohdr_t *(__stdcall* oGetStudiomodel)(const model_t*);
		return getvfunc< oGetStudiomodel >(this, 32)(mod);
	}
};

class IVModelInfoClient : public IVModelInfo
{
public:
};

struct virtualterrainparams_t
{
	int index;
};