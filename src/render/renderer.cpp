#include "i_render.h"

Renderer::Renderer()
: mWorldProjector(-1.0f,1.0f)
, mUiProjector(0.0f,100.0f,Projection::VM_Fixed)
, mCamera(mWorldProjector)
, mUi(Ui::Get())
, mDecalEngine(DecalEngine::Get())
, mShaderManager(ShaderManager::Get())
{
	Font::Get();
	mMouseMoveId=EventServer<ScreenMouseMoveEvent>::Get().Subscribe(boost::bind(&Renderer::OnMouseMoveEvent,this,_1));
	mMousePressId=EventServer<ScreenMousePressEvent>::Get().Subscribe(boost::bind(&Renderer::OnMousePressEvent,this,_1));
	mMouseReleaseId=EventServer<ScreenMouseReleaseEvent>::Get().Subscribe(boost::bind(&Renderer::OnMouseReleaseEvent,this,_1));
	Init();
}

Renderer::~Renderer()
{

}

bool Renderer::Render()
{
	if(!BeginRender()) return false;
	mCamera.Update();

	// render world
	SetupRenderer(mUiProjector);
	Scene& Scen(Scene::Get());
	mSceneRenderer.Draw(Scen);
	mDecalEngine.Draw(DecalEngine::GroundParticle);
	mActorRenderer.Draw(Scen);
	mUiRenderer.Draw(mUi.GetRoot());

	return EndRender();
}

void Renderer::SetupRenderer(const Projection& Proj)
{
	Viewport const& Vp=Proj.GetViewport();
	glViewport(Vp.X, Vp.Y, Vp.Width, Vp.Height);

	mShaderManager.UploadGlobalData(GlobalShaderData::WorldProjection,mWorldProjector.GetMatrix());
	mShaderManager.UploadGlobalData(GlobalShaderData::WorldCamera,mCamera.GetView());
	mShaderManager.UploadGlobalData(GlobalShaderData::UiProjection,mUiProjector.GetMatrix());
}

bool Renderer::BeginRender()
{
	glClear(GL_COLOR_BUFFER_BIT);
	return true;
}

bool Renderer::EndRender()
{
	return true;
}

void Renderer::OnMouseMoveEvent( const ScreenMouseMoveEvent& Event )
{
	glm::vec3 EvtPos(Event.Pos.x,Event.Pos.y,0);
	glm::vec3 UiEvtPos=mUiProjector.Unproject(EvtPos);
	UiMouseMoveEvent UiEvt(glm::vec2(UiEvtPos.x,UiEvtPos.y));
	if(EventServer<UiMouseMoveEvent>::Get().SendEvent(UiEvt))return;

	glm::vec3 WorldEvtPos(mCamera.GetInverseView()*glm::vec4(mWorldProjector.Unproject(EvtPos),1.0));
	WorldMouseMoveEvent WorldEvt(glm::vec2(WorldEvtPos.x,WorldEvtPos.y));
	EventServer<WorldMouseMoveEvent>::Get().SendEvent(WorldEvt);
}

void Renderer::OnMousePressEvent( const ScreenMousePressEvent& Event )
{
	glm::vec3 EvtPos(Event.Pos.x,Event.Pos.y,0);
	glm::vec3 UiEvtPos=mUiProjector.Unproject(EvtPos);
	UiMousePressEvent UiEvt(glm::vec2(UiEvtPos.x,UiEvtPos.y),Event.Button);
	if(EventServer<UiMousePressEvent>::Get().SendEvent(UiEvt))return;

	glm::vec3 WorldEvtPos(mCamera.GetInverseView()*glm::vec4(mWorldProjector.Unproject(EvtPos),1.0));
	WorldMousePressEvent WorldEvt(glm::vec2(WorldEvtPos.x,WorldEvtPos.y),Event.Button);
	EventServer<WorldMousePressEvent>::Get().SendEvent(WorldEvt);
}

void Renderer::OnMouseReleaseEvent(const ScreenMouseReleaseEvent& Event)
{
	glm::vec3 EvtPos(Event.Pos.x,Event.Pos.y,0);
	glm::vec3 UiEvtPos=mUiProjector.Unproject(EvtPos);
	UiMouseReleaseEvent UiEvt(glm::vec2(UiEvtPos.x,UiEvtPos.y),Event.Button);
	if(EventServer<UiMouseReleaseEvent>::Get().SendEvent(UiEvt))return;

	glm::vec3 WorldEvtPos(mCamera.GetInverseView()*glm::vec4(mWorldProjector.Unproject(EvtPos),1.0));
	WorldMouseReleaseEvent WorldEvt(glm::vec2(WorldEvtPos.x,WorldEvtPos.y),Event.Button);
	EventServer<WorldMouseReleaseEvent>::Get().SendEvent(WorldEvt);
}

void Renderer::Init()
{
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}

