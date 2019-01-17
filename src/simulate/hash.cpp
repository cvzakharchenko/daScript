#include "daScript/misc/platform.h"

#include "daScript/simulate/hash.h"
#include "daScript/simulate/runtime_string.h"

namespace das
{
	// ideas from http://isthe.com/chongo/tech/comp/fnv/

	uint32_t hash_block32(uint8_t * block, size_t size) {
		const uint32_t fnv_prime = 16777619;
		const uint32_t fnv_bias = 2166136261;
		uint32_t offset_basis = fnv_bias;
		for (; size; size--, block++) {
			offset_basis = offset_basis * fnv_prime ^ *block;
		}
		assert(offset_basis != HASH_EMPTY32 && offset_basis != HASH_KILLED32);
		if (offset_basis == HASH_EMPTY32 && offset_basis == HASH_KILLED32) {
			return fnv_prime;
		}
		return offset_basis;
	}

	uint32_t hash_blockz32(uint8_t * block) {
		const uint32_t fnv_prime = 16777619;
		const uint32_t fnv_bias = 2166136261;
		uint32_t offset_basis = fnv_bias;
		for ( ; *block ; block++) {
			offset_basis = offset_basis * fnv_prime ^ *block;
		}
		assert(offset_basis != HASH_EMPTY32 && offset_basis != HASH_KILLED32);
		if (offset_basis == HASH_EMPTY32 && offset_basis == HASH_KILLED32) {
			return fnv_prime;
		}
		return offset_basis;
	}

	uint64_t hash_block64(uint8_t * block, size_t size) {
		const uint64_t fnv_prime = 1099511628211UL;
		const uint64_t fnv_bias = 14695981039346656037UL;
		uint64_t offset_basis = fnv_bias;
		for (; size; size--, block++) {
			offset_basis = offset_basis * fnv_prime ^ *block;
		}
		assert(offset_basis != HASH_EMPTY64 && offset_basis != HASH_KILLED64);
		if (offset_basis == HASH_EMPTY64 && offset_basis == HASH_KILLED64) {
			return fnv_prime;
		}
		return offset_basis;
	}
    
    uint64_t hash_blockz64(uint8_t * block) {
        const uint64_t fnv_prime = 1099511628211UL;
        const uint64_t fnv_bias = 14695981039346656037UL;
        uint64_t offset_basis = fnv_bias;
        for ( ; *block ; block++) {
            offset_basis = offset_basis * fnv_prime ^ *block;
        }
		assert(offset_basis != HASH_EMPTY64 && offset_basis != HASH_KILLED64);
		if (offset_basis == HASH_EMPTY64 && offset_basis == HASH_KILLED64) {
			return fnv_prime;
		}
        return offset_basis;
    }

    __forceinline uint32_t _rotl ( uint32_t value, int shift ) {
        return (value<<shift) | (value>>(32-shift));
    }
    
	uint32_t hash_value ( void * pX, TypeInfo * info );
    
	uint32_t hash_structure ( char * ps, StructInfo * info, int size, bool isPod ) {
        if ( isPod ) {
            return hash_function(ps, size);
        } else {
			uint32_t hash = 0;
            char * pf = ps;
            for ( uint32_t i=0; i!=info->fieldsSize; ++i ) {
                VarInfo * vi = info->fields[i];
                hash = _rotl(hash,2) ^ hash_value(pf, vi);
                pf += getTypeSize(vi);
            }
            return hash;
        }
    }
    
	uint32_t hash_array_value ( void * pX, int stride, int count, TypeInfo * info ) {
        if ( info->isPod ) {
            return hash_function(pX, stride * count);
        } else {
			uint32_t hash = 0;
            char * pA = (char *) pX;
            for ( int i=0; i!=count; ++i ) {
                hash = _rotl(hash,2) ^ hash_value(pA, info);
                pA += stride;
            }
            return hash;
        }
    }
    
	uint32_t hash_dim_value ( void * pX, TypeInfo * info ) {
        TypeInfo copyInfo = *info;
        assert(copyInfo.dimSize);
        copyInfo.dimSize --;
        int stride = getTypeBaseSize(info);
        int count = getDimSize(info);
        return hash_array_value(pX, stride, count, &copyInfo);
    }
    
	uint32_t hash_value ( void * pX, TypeInfo * info ) {
        if ( info->ref ) {
            TypeInfo ti = *info;
            ti.ref = false;
            return hash_value(*(void **)pX, &ti);
        } else if ( info->dimSize ) {
            return hash_dim_value(pX, info);
        } else if ( info->type==Type::tArray ) {
            auto arr = (Array *) pX;
            return hash_array_value(arr->data, getTypeSize(info->firstType), arr->size, info->firstType);
        } else {
            switch ( info->type ) {
                case Type::tBool:       return hash_function(*((bool *)pX));
                case Type::tInt64:      return hash_function(*((int64_t *)pX));
                case Type::tUInt64:     return hash_function(*((uint64_t *)pX));
                case Type::tString:     return hash_function(safe_str(pX));
                case Type::tInt:        return hash_function(*((int32_t *)pX));
                case Type::tInt2:       return hash_function(*((int2 *)pX));
                case Type::tInt3:       return hash_function(*((int3 *)pX));
                case Type::tInt4:       return hash_function(*((int4 *)pX));
                case Type::tUInt:       return hash_function(*((uint32_t *)pX));
                case Type::tUInt2:      return hash_function(*((uint2 *)pX));
                case Type::tUInt3:      return hash_function(*((uint3 *)pX));
                case Type::tUInt4:      return hash_function(*((uint4 *)pX));
                case Type::tFloat:      return hash_function(*((float *)pX));
                case Type::tFloat2:     return hash_function(*((float2 *)pX));
                case Type::tFloat3:     return hash_function(*((float3 *)pX));
                case Type::tFloat4:     return hash_function(*((float4 *)pX));
                case Type::tRange:      return hash_function(*((range *)pX));
                case Type::tURange:     return hash_function(*((urange *)pX));
                case Type::tPointer:    return hash_function(intptr_t(pX));
                case Type::tIterator:   return hash_function(intptr_t(pX));
                case Type::tStructure:  return hash_structure(*(char **)pX, info->structType, getTypeSize(info), info->isPod);
                default:                assert(0 && "unsupported print type"); return 0;
            }
        }
    }
    
	uint32_t hash_function ( void * data, TypeInfo * type ) {
        return hash_value(data, type);
    }
}

