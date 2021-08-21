#pragma once
#include <string>
#include <time.h>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <map>
#include <vector>

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p) = NULL; } }
#define SAFE_DELETE(x) { if(x) delete x; x = NULL; }

#define D3DFONT_ITALIC (1 << 0)
#define D3DFONT_ZENABLE (1 << 1)
#define D3DFVF_FONT2DVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
class Vector2D;
