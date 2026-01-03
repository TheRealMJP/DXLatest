float4 PSMain(float4 position : SV_Position, float3 color : COLOR) : SV_Target0
{
    return float4(color, 1.0f);
}