#pragma once

#include "simulate.h"
#include "function_traits.h"

namespace yzg
{
    // convert arguments into a tuple of appropriate types
    template <typename ArgumentsType, size_t... I>
    __forceinline auto cast_args (__m128 * args, index_sequence<I...> ) {
        return make_tuple( cast< typename tuple_element<I, ArgumentsType>::type  >::to ( args[ I ] )... );
    }
    // void static function or lambda
    template <typename Result>
    struct ImplCallStaticFunction {
        template <typename FunctionType, typename TupleType, size_t... I>
        static __forceinline __m128 call(FunctionType & fn, TupleType && args, index_sequence<I...> ) {
            return cast<Result>::from ( fn(get<I>(forward<TupleType>(args))...) );
        }
    };
    
    template <>
    struct ImplCallStaticFunction<void> {
        template <typename FunctionType, typename TupleType, size_t... I>
        static __forceinline __m128 call(FunctionType & fn, TupleType && args, index_sequence<I...> ) {
            fn(get<I>(forward<TupleType>(args))...);
            return _mm_setzero_ps();
        }
    };
    
    template <typename FuncT, FuncT fn>
    struct SimNode_ExtFuncCall : SimNode_Call
    {
        SimNode_ExtFuncCall ( const LineInfo & at ) : SimNode_Call(at) {}
        virtual __m128 eval ( Context & context ) override {
            using FunctionTrait = function_traits<FuncT>;
            using Result = typename FunctionTrait::return_type;
            const int nargs = tuple_size<typename FunctionTrait::arguments>::value;
            using Indices = make_index_sequence<nargs>;
            using Arguments = typename FunctionTrait::arguments;
            evalArgs(context);
            YZG_EXCEPTION_POINT;
            __m128 * args = abiArgValues(context);
            auto cpp_args = cast_args<Arguments>(args, Indices());
        #if YZG_ENABLE_STACK_WALK
            // PUSH
            if ( context.stack - ( context.stackTop - sizeof(Prologue) ) > context.stackSize ) {
                context.throw_error("stack overflow");
                return _mm_setzero_ps();
            }
            char * pushStack = context.stackTop;
            context.stackTop -= sizeof(Prologue);
            // fill prologue
            Prologue * pp = (Prologue *) context.stackTop;
            pp->result =        _mm_setzero_ps();
            pp->arguments =     args;
            pp->info =          info;
            pp->line =          debug.line;
        #endif
            // calc
            auto res = ImplCallStaticFunction<Result>::call(*fn, move(cpp_args), Indices());
            // POP
        #if YZG_ENABLE_STACK_WALK
            context.stackTop = pushStack;
        #endif
            return res;
        }
        FuncInfo * info = nullptr;
    };
    
    typedef __m128 ( InteropFunction ) ( Context & context, SimNode_Call * node, __m128 * args );
    
    template <InteropFunction fn>
    struct SimNode_InteropFuncCall : SimNode_Call
    {
        SimNode_InteropFuncCall ( const LineInfo & at ) : SimNode_Call(at) {}
        virtual __m128 eval ( Context & context ) override {
            evalArgs(context);
            YZG_EXCEPTION_POINT;
            __m128 * args = abiArgValues(context);
#if YZG_ENABLE_STACK_WALK
            // PUSH
            if ( context.stack - ( context.stackTop - sizeof(Prologue) ) > context.stackSize ) {
                context.throw_error("stack overflow");
                return _mm_setzero_ps();
            }
            char * pushStack = context.stackTop;
            context.stackTop -= sizeof(Prologue);
            // fill prologue
            Prologue * pp = (Prologue *) context.stackTop;
            pp->result =        _mm_setzero_ps();
            pp->arguments =     args;
            pp->info =          info;
            pp->line =          debug.line;
#endif
            // calc
            auto res = fn(context,this,args);
            // POP
#if YZG_ENABLE_STACK_WALK
            context.stackTop = pushStack;
#endif
            return res;
        }
        FuncInfo * info = nullptr;
    };
}

