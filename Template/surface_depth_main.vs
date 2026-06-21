void main()
{
    MaterialVertexInputs material;
    initMaterialVertex(material);
    materialVertex(material);
    gl_Position = getClipFromWorldMatrix() * getWorldPosition(material);
}
