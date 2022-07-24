#include "Lights.hpp"
#include "Aurora/Engine.hpp"
#include "Aurora/Graphics/Base/IRenderDevice.hpp"
#include "Aurora/Graphics/DShape.hpp"

#include "CameraComponent.hpp"

// Implemented from https://doc.magnum.graphics/magnum/examples-shadows.html

namespace Aurora
{

	void DirectionalLightComponent::SetupShadowmaps(int32 numShadowLevels, const Vector2i& resolution)
	{
		Layers.clear();

		TextureDesc textureDesc;
		textureDesc.Width = resolution.x;
		textureDesc.Height = resolution.y;
		textureDesc.DimensionType = EDimensionType::TYPE_2DArray;
		textureDesc.ImageFormat = GraphicsFormat::D32;
		textureDesc.MipLevels = 1;
		textureDesc.DepthOrArraySize = numShadowLevels;
		RenderTexture = GEngine->GetRenderDevice()->CreateTexture(textureDesc);

		for(std::int_fast32_t i = 0; i < numShadowLevels; ++i)
		{
			Layers.emplace_back(resolution);
			ShadowMatrices.emplace_back(glm::identity<Matrix4>());
		}
	}

	void DirectionalLightComponent::SetTarget(CameraComponent* mainCamera)
	{
		//Matrix4 lightCameraMatrix = glm::lookAt({0, 0, 0}, -GetLeftVector(), Vector3(0, 1, 0));
		Matrix4 lightCameraMatrix = GetTransformationMatrix();
		const Matrix3 cameraRotationMatrix = Matrix3(lightCameraMatrix);
		const Matrix3 inverseCameraRotationMatrix = glm::inverse(cameraRotationMatrix);

		//DShapes::ScreenText(Vector2(5, 200), glm::to_string(cameraRotationMatrix));

		for (std::size_t layerIndex = 0; layerIndex != Layers.size(); ++layerIndex)
		{
			std::vector<Vector3> mainCameraFrustumCorners = LayerFrustumCorners(mainCamera, int(layerIndex));
			ShadowLayerData& layer = Layers[layerIndex];

			/* Calculate the AABB in shadow-camera space */
			Vector3 min(std::numeric_limits<float>::max());
			Vector3 max(std::numeric_limits<float>::lowest());
			for (Vector3 worldPoint: mainCameraFrustumCorners)
			{
				//Vector3 cameraPoint = worldPoint;
				Vector3 cameraPoint = inverseCameraRotationMatrix * worldPoint;
				min = glm::min(min, cameraPoint);
				max = glm::max(max, cameraPoint);

				//DShapes::WireBox(AABB(worldPoint - Vector3(2.0f), worldPoint + Vector3(2.0f)));
			}

			/* Place the shadow camera at the mid-point of the camera box */
			const Vector3 mid = (min + max) * 0.5f;
			const Vector3 cameraPosition = cameraRotationMatrix * mid;

			//DShapes::WireBox(AABB(cameraPosition - Vector3(2.0f), cameraPosition + Vector3(2.0f)), Color::blue());
			//DShapes::WireBox(AABB(mid - Vector3(2.0f), mid + Vector3(2.0f)), Color::red());

			const Vector3 range = max - min;
			/* Set up the initial extends of the shadow map's render volume. Note
			   we will adjust this later when we render. */
			layer.OrthographicSize = Vector2i(range.x, range.y);
			layer.OrthographicNear = -0.5f * range.z;
			layer.OrthographicFar =  0.5f * range.z;
			lightCameraMatrix[3] = Vector4(cameraPosition, lightCameraMatrix[3].w);
			layer.ShadowCameraMatrix = lightCameraMatrix;
		}
	}

	void DirectionalLightComponent::SetupSplitDistances(const float zNear, const float zFar, const float power) {
		/* props http://stackoverflow.com/a/33465663 */
		for(std::size_t i = 0; i != Layers.size(); ++i)
		{
			const float linearDepth = zNear + std::pow(float(i + 1)/Layers.size(), power)*(zFar - zNear);
			const float nonLinearDepth = (zFar + zNear - 2.0f*zNear*zFar/linearDepth)/(zFar - zNear);
			Layers[i].CutPlane = (nonLinearDepth + 1.0f)/2.0f;
			AU_LOG_INFO("Plane: ", Layers[i].CutPlane, ", ", nonLinearDepth, ", ", linearDepth);
		}
	}

	std::vector<Vector3> DirectionalLightComponent::LayerFrustumCorners(CameraComponent* mainCamera, const int layer)
	{
		const float z0 = layer == 0 ? 0 : Layers[layer - 1].CutPlane;
		const float z1 = Layers[layer].CutPlane;
		return CameraFrustumCorners(mainCamera, z0, z1);
	}
	std::vector<Vector3> DirectionalLightComponent::CameraFrustumCorners(CameraComponent* mainCamera, const float z0, const float z1)
	{
		const Matrix4 imvp = glm::inverse(mainCamera->GetProjectionViewMatrix());
		return FrustumCorners(imvp, z0, z1);
	}

	Vector3 TransformPoint(const Matrix4& matrix, const Vector3& point)
	{
		const Vector4 transformed = matrix * Vector4(point, 1.0f);
		return {
			transformed.x / transformed.w,
			transformed.y / transformed.w,
			transformed.z / transformed.w
		};
	}

	std::vector<Vector3> DirectionalLightComponent::FrustumCorners(const Matrix4& imvp, const float z0, const float z1)
	{
		return {TransformPoint(imvp, {-1,-1, z0}),
		        TransformPoint(imvp, { 1,-1, z0}),
		        TransformPoint(imvp, {-1, 1, z0}),
		        TransformPoint(imvp, { 1, 1, z0}),
		        TransformPoint(imvp, {-1,-1, z1}),
		        TransformPoint(imvp, { 1,-1, z1}),
		        TransformPoint(imvp, {-1, 1, z1}),
		        TransformPoint(imvp, { 1, 1, z1})};
	}

	std::vector<Vector4> DirectionalLightComponent::CalculateClipPlanes(const Matrix4& lightProjection) {
		const Matrix4 pm = lightProjection;
		std::vector<Vector4> clipPlanes
		{
			{pm[3][0] + pm[2][0], pm[3][1] + pm[2][1], pm[3][2] + pm[2][2], pm[3][3] + pm[2][3]},   /* near */
			{pm[3][0] - pm[2][0], pm[3][1] - pm[2][1], pm[3][2] - pm[2][2], pm[3][3] - pm[2][3]},   /* far */
			{pm[3][0] + pm[0][0], pm[3][1] + pm[0][1], pm[3][2] + pm[0][2], pm[3][3] + pm[0][3]},   /* left */
			{pm[3][0] - pm[0][0], pm[3][1] - pm[0][1], pm[3][2] - pm[0][2], pm[3][3] - pm[0][3]},   /* right */
			{pm[3][0] + pm[1][0], pm[3][1] + pm[1][1], pm[3][2] + pm[1][2], pm[3][3] + pm[1][3]},   /* bottom */
			{pm[3][0] - pm[1][0], pm[3][1] - pm[1][1], pm[3][2] - pm[1][2], pm[3][3] - pm[1][3]}    /* top */
		};

		for(Vector4& plane: clipPlanes)
		{
			plane *= 1.0f / glm::length(Vector3(plane));
		}
		return clipPlanes;
	}

	void DirectionalLightComponent::Render(DrawCallState& drawCallState, const LightRenderFnc& renderCallback)
	{
		/* Projecting world points normalized device coordinates means they range
					       -1 -> 1. Use this bias matrix so we go straight from world -> texture
					       space */
		constexpr const Matrix4 bias{{0.5f, 0.0f, 0.0f, 0.0f},
		                             {0.0f, 0.5f, 0.0f, 0.0f},
		                             {0.0f, 0.0f, 1.0f, 0.0f},
		                             {0.5f, 0.5f, 0.0f, 1.0f}};

		/*constexpr const Matrix4 unbiasMatrix{{ 2.0f,  0.0f,  0.0f, 0.0f},
		                                     { 0.0f,  2.0f,  0.0f, 0.0f},
		                                     { 0.0f,  0.0f,  2.0f, 0.0f},
		                                     {-1.0f, -1.0f, -1.0f, 1.0f}};*/

		CameraComponent lightCamera;

		for (std::size_t layer = 0; layer != Layers.size(); layer++)
		{
			ShadowLayerData& d = Layers[layer];
			float orthographicNear = d.OrthographicNear;
			const float orthographicFar = d.OrthographicFar;

			//const std::vector<Vector4> clipPlanes = CalculateClipPlanes(lightProjection);

			float hx = d.OrthographicSize.x / 2.0f;
			float hy = d.OrthographicSize.y / 2.0f;

			lightCamera.SetProjectionMatrix(glm::ortho(-hx, hx, hy, -hy, orthographicNear, orthographicFar));
			lightCamera.GetTransform().SetFromMatrix(d.ShadowCameraMatrix);
			lightCamera.UpdateFrustum();

			Matrix4 invertedShadowCameraMatrix = glm::inverse(d.ShadowCameraMatrix);
			ShadowMatrices[layer] = bias * lightCamera.GetProjectionMatrix() * invertedShadowCameraMatrix;

			renderCallback(&lightCamera, &lightCamera.GetFrustum(), invertedShadowCameraMatrix, int(layer));
		}
	}
}
