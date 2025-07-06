#include "Model.h"

Model::Model(const string& path, bool gamma)
	: b_GammaCorrection(gamma)
{
	LoadModel(path);
}
