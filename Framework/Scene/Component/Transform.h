#pragma once
#include "IComponent.h"

class Transform final : public IComponent
{
public:
	Transform
	(
		class Context* context,
		class Actor* actor,
		class Transform* transform
	);
	~Transform() = default;

	Transform(const Transform&) = delete;
	Transform& operator=(const Transform&) = delete;

	void Serialize(FileStream* stream) override;
	void Deserialize(FileStream* stream) override;

	void OnInitialize() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;
	void OnDestroy() override;

	//=======================================================================
	//Local
	const D3DXVECTOR3& GetLocalScale() const { return localScale; }
	const D3DXQUATERNION& GetLocalRotation() const { return localRotation; }
	const D3DXVECTOR3& GetLocalTranslation() const { return localTranslation; }
	const D3DXMATRIX& GetLocalMatrix() const { return local; }

	void SetLocalScale(const D3DXVECTOR3& vec);
	void SetLocalRotation(const D3DXQUATERNION& qut);
	void SetLocalTranslation(const D3DXVECTOR3& vec);
	//=======================================================================

	//=======================================================================
	//World
	const D3DXVECTOR3 GetScale();
	const D3DXQUATERNION GetRotation();
	const D3DXVECTOR3 GetTranslation();
	const D3DXMATRIX& GetWorldMatrix() const { return world; }
	const D3DXMATRIX GetWorldRotationMatrix();

	void SetScale(const D3DXVECTOR3& vec);
	void SetRotation(const D3DXQUATERNION& qut);
	void SetRotationFromEulerAngle(const D3DXVECTOR3& eulerAngle);
	void SetTranslation(const D3DXVECTOR3& vec);
	//=======================================================================

	//=======================================================================
	//Direction
	const D3DXVECTOR3 GetRight();
	const D3DXVECTOR3 GetUp();
	const D3DXVECTOR3 GetForward();
	//=======================================================================

	//=======================================================================
	//Translate
	void Translate(const D3DXVECTOR3& delta);
	//=======================================================================

	//=======================================================================
	//Transform
	Transform* GetRoot() { return HasParent() ? parent->GetRoot() : this; }
	Transform* GetParent() const { return parent; }
	auto GetChilds() const -> const std::vector<Transform*>& { return childs; }
	auto GetChildFromIndex(const uint& index)->Transform*;
	auto GetChildCount() const -> const uint { return childs.size(); }

	void SetParent(Transform* newParent);
	void AddChild(Transform* child);
	void Detatch();
	void AcquireChilds();

	const bool HasParent() const { return parent ? true : false; }
	const bool HasChilds() const { return !childs.empty(); }
	const bool IsChild(Transform* transform);
	//=======================================================================

	const D3DXVECTOR3 GetEulerAngle();

	auto GetConstantBuffer() const -> class ConstantBuffer* { return gpu_transform_buffer.get(); }
	void UpdateConstantBuffer(const D3DXMATRIX& vp_matrix);

	void UpdateTransform();

private:
	TransformData cpu_transform_buffer;
	std::shared_ptr<class ConstantBuffer> gpu_transform_buffer;

	D3DXVECTOR3 localScale;
	D3DXQUATERNION localRotation;
	D3DXVECTOR3 localTranslation;

	D3DXMATRIX local;
	D3DXMATRIX world;
	D3DXMATRIX wvp_previous;

	Transform* parent;
	std::vector<Transform*> childs;
};