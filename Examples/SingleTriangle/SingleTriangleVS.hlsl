struct VSOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

VSOutput VSMain(uint vertexID : SV_VertexID)
{
    float posX = -0.5f + vertexID * 0.5f;
    float posY = -0.25f + (vertexID % 2);

    VSOutput output;
    output.Position = float4(posX, posY, 1.0f, 1.0f);

    if (vertexID == 0)
        output.Color = float3(1, 0, 0);
    else if (vertexID == 1)
        output.Color = float3(0, 1, 0);
    else
        output.Color = float3(0, 0, 1);

    return output;
}