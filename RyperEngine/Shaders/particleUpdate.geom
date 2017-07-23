#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

in float Type0[];
in vec3 Position0[];
in vec3 Velocity0[];
in float Age0[];
in vec3 Color0[];

out float Type1;
out vec3 Position1;
out vec3 Velocity1;
out float Age1;
out vec3 Color1;

uniform float gDeltaTimeMillis;
uniform float gTime;
uniform float gRandom;
uniform sampler1D gRandomTexture;
uniform float gLauncherLifetime = 0.8f;
uniform float gShellLifetime    = 3.0f;
uniform float gSecondaryShellLifetime = 8.0f;

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

vec3 GetRandomDir(float TexCoord)
{
    vec3 Dir = texture(gRandomTexture, TexCoord).xyz; //[0, 1]
    Dir = Dir - vec3(0.5); //[-0.5, 0.5]
    return Dir;
}

vec3 GetRandomColor(float TexCoord)
{
	vec3 color = texture(gRandomTexture, TexCoord).xyz; //[0, 1]
    return color;
}

void main()                                                                         
{                                                                                   
    float Age = Age0[0] + gDeltaTimeMillis;
	vec3 OrigColor = Color0[0];                                       
                                                                                    
    if (Type0[0] == PARTICLE_TYPE_LAUNCHER) {                                       
        if (Age >= gLauncherLifetime) {                                             
            Type1 = PARTICLE_TYPE_SHELL;                                            
            Position1 = Position0[0];                                               
            vec3 Dir = GetRandomDir(gRandom);                                  
            Dir.y = max(Dir.y, 1.0);
            Velocity1 = normalize(Dir) * 3;
			Color1 = GetRandomColor(gRandom);                                
            Age1 = 0.0;                                                       
			EmitVertex();                                                           
			EndPrimitive();                                                         
			Age = 0.0;                                                              
        }                                                                           
                                                                                    
        Type1 = PARTICLE_TYPE_LAUNCHER;                                             
        Position1 = Position0[0];                                                   
        Velocity1 = Velocity0[0];                                                   
        Age1 = Age;
		Color1 = OrigColor;                                                    
        EmitVertex();                                                               
        EndPrimitive();                                                             
    }                                                                               
    else {                                                                          
        float DeltaTimeSecs = gDeltaTimeMillis;                                                         
        vec3 DeltaP = DeltaTimeSecs * Velocity0[0];                                 
        vec3 DeltaV = vec3(DeltaTimeSecs) * (0.0, -9.81, 0.0) * 10;                      
                                                                                    
        if (Type0[0] == PARTICLE_TYPE_SHELL)  {                                     
	        if (Age < gShellLifetime) {                                             
	            Type1 = PARTICLE_TYPE_SHELL;                                        
	            Position1 = Position0[0] + DeltaP;                                  
	            Velocity1 = Velocity0[0] + DeltaV;                                  
	            Age1 = Age;
				Color1 = OrigColor;
	            EmitVertex();                                                       
	            EndPrimitive();                                                     
	        }                                                                       
            else {               
				vec3 newColor = GetRandomColor(gTime + gRandom);
                for (int i = 0 ; i < 10 ; i++) {                                    
                     Type1 = PARTICLE_TYPE_SECONDARY_SHELL;                         
                     Position1 = Position0[0];                                      
                     vec3 Dir = GetRandomDir((gTime + i/12.0f));                   
					 Dir.y = max(Dir.y, 0.5);
                     Velocity1 = normalize(Dir);                             
					 Color1 = newColor;
                     Age1 = 0.0f;                                       
                     EmitVertex();                                                  
                     EndPrimitive();                                                
                }                                                                   
            }                                                                       
        }                                                                           
        else {                                                                      
            if (Age < gSecondaryShellLifetime) {                                    
                Type1 = PARTICLE_TYPE_SECONDARY_SHELL;                              
                Position1 = Position0[0] + DeltaP;                                  
                Velocity1 = Velocity0[0] + DeltaV;                                  
                Age1 = Age;    
				Color1 = OrigColor;                                                     
                EmitVertex();                                                       
                EndPrimitive();                                                     
            }                                                                       
        }                                                                           
    }                                                                               
} 