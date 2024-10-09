#version 400

uniform vec3 lightPosition;

/**
 * --Task--
 * Receive the texture uniform.
 * - Find out what type is it
 * - You can use any name for it, because there is only one texture at a time
 */
uniform sampler2D texture

in vec3 interpolatedNormal;
in vec3 interpolatedPosition;
in vec2 interpolatedUv;
in vec3 interpolatedColor;

out vec4 fragColor;

void main(void) {
    vec3 viewerPosition = vec3(0.0);
    float shininess = 200.0f;
    
    vec3 color = interpolatedColor; //Assign your calculation here instead 
    vec3 n = normalize(interpolatedNormal); 
    vec3 vertexPosition = interpolatedPosition;

    vec3 v = normalize(-vertexPosition);
    vec3 l = normalize(lightPosition - vertexPosition);
    vec3 r = normalize(reflect(-l, n));
    vec3 h = normalize(l + v); 


    vec3 ambientDiffuse = color * (vec3(0.5) + vec3(0.5) * max(dot(n,l) ,0.0));

    vec3 specularHighlight = vec3(pow(max(0.0, dot(h,n)), shininess));
    fragColor = vec4(specularHighlight+ambientDiffuse, 1.0);
    /**
     * --Task--
     * Copy your Phong or Blinn lighting model here.
     * Use the diffuse and ambient color from the texture - sample it from the correct coordinates.
     */
}
