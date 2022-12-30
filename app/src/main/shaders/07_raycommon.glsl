struct HitPayload
{
	vec4 hitValue;
	bool isMiss;
};

struct Vertex3D
{
    vec3 pos;
    vec3 color;
    vec3 normal;
};

struct Vertex3DObj
{
  vec3 pos;
  vec3 normal;
  vec2 texcoord;
  vec4 vertexTangent;
};

struct PayroadBlend
{
  vec3 pos;
  vec3 color;
  vec3 normal;
  bool hit;
  bool isMiss;
};

struct AnimationUniforms
{
  uint animNum;
  float time;
  float scale;
  uint vertexSize;
  uint keyFramesSize;
  uint morphTargetSize;
  uint morphTargetPositionSize;
};

struct GltfMaterial{
  float alphaCutoff;
};

struct WaveUBO{
  float seabase;
  float dz;
  float top;
  float right;
  float paddle;
  float speed;
  float freq;
  float amp;
  float time;
};

struct Light{
  vec3 lightPosition;
  vec3 eyeDirection;
  vec3 ambientColor;
  float intensity;
};

float nAir = 1.000;
float nWater = 1.333;
float nGlass = 1.55;

float PURPLE = 0.4;
float INDIGO = 0.47;
float BLUE = 0.5;
float GREEN = 0.57;
float YELLOW = 0.6;
float ORANGE = 0.65;
float RED = 0.78;
//Sellmeire Glass
float K1 = 1.737;
float L1 = 1.319 * 0.01;
float K2 = 3.137 * 0.1;
float L2 = 6.231 * 0.01;
float K3 = 1.899;
float L3 = 1.552 * 0.01;
const float WIDTH = 1920.0;
const float HEIGHT = 1080.0;
const vec3 WATER_COLOR = vec3(0.0, 0.5, 96.0 / 255.0);

float Sellmeire(float lambda)
{
  float l2 = lambda * lambda;
  float n2 = ((K1 * l2) / (l2 - L1)) + ((K2 * l2) / (l2 - L2)) + ((K3 * l2) / (l2 - L3)) + 1;
  return sqrt(n2);
}

struct PayloadRefract
{
  vec3 pos;
  vec3 color;
  vec3 direction;
  float refractiveIndex;
  uint objId;
};

float ReflectanceP(vec3 incident, vec3 normal, float fromN, float toN)
{
  vec3 incidentN = normalize(incident);
  vec3 normalN = normalize(normal);
  vec3 refractVec = refract(incidentN, normalN, fromN / toN);
  float ci = dot(-incidentN, normalN);
  float ct = dot(refractVec, -normalN);
  float n1ci = toN * ci;
  float n0ct = fromN * ct;
  float I = (n0ct - n1ci) / (n0ct + n1ci);
  return I * I;
}

float ReflectanceS(vec3 incident, vec3 normal, float fromN, float toN)
{
  vec3 incidentN = normalize(incident);
  vec3 normalN = normalize(normal);
  vec3 refractVec = refract(incidentN, normalN, fromN / toN);
  float ci = dot(-incidentN, normalN);
  float ct = dot(refractVec, -normalN);
  float n0ci = fromN * ci;
  float n1ct = toN * ct;
  float I = (n0ci - n1ct) / (n0ci + n1ct);
  return I * I;
}

struct PayloadVecFloat
{
  vec4 pos;
  float value;
};

