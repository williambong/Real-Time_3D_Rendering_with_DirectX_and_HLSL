#pragma once

#include "Common.h"

namespace Library
{
    class Effect;

    class Variable
    {
    public:
        Variable(Effect& effect, ID3DX11EffectVariable* variable);

        Effect& GetEffect();
        ID3DX11EffectVariable* GetVariable() const;
        const D3DX11_EFFECT_VARIABLE_DESC& VariableDesc() const;
        ID3DX11EffectType* Type() const;
        const D3DX11_EFFECT_TYPE_DESC& TypeDesc() const;
        const std::string& Name() const;

		//TODO: make this std::unique_ptr
		Variable operator[](UINT index);

        Variable& operator<<(CXMMATRIX value);
        Variable& operator<<(ID3D11ShaderResourceView* value);
		Variable& operator<<(ID3D11UnorderedAccessView* value);
        Variable& operator<<(FXMVECTOR value);
        Variable& operator<<(int value);
		Variable& operator<<(float value);
		Variable& operator<<(const std::vector<float>& values);
		Variable& operator<<(const std::vector<XMFLOAT2>& values);
		Variable& operator<<(const std::vector<XMFLOAT4X4>& values);

    private:
		//TODO: remove
        //Variable(const Variable& rhs);
        Variable& operator=(const Variable& rhs);

        Effect& mEffect;
        ID3DX11EffectVariable* mVariable;
        D3DX11_EFFECT_VARIABLE_DESC mVariableDesc;
        ID3DX11EffectType* mType;
        D3DX11_EFFECT_TYPE_DESC mTypeDesc;
        std::string mName;
    };
}