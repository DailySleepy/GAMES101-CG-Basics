
for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
 
 
 
        //光照方向的反向量,normalized()的作用是把向量归一化，也就是方向不变，但模长变成为1
        Eigen::Vector3f light_direction = (light.position - point).normalized();
 
        //视线向量
        Eigen::Vector3f view_direction = (eye_pos - point).normalized();
 
        //两个向量加起来归一化得到半程向量
        Eigen::Vector3f half_vector = (light_direction + view_direction).normalized();
 
        //视角与反射角之间存在一定的夹角造成了光强的衰减，影响反射强度,如果点乘出来的结果为负数，说明视线与法线不在同侧，就看不到该像素点，反射强度就为0
        float reflection_intensity = std::max(0.0f, half_vector.dot(normal));
 
        //计算随着距离增大造成的光线强度的衰减，即衰减系数,它是光源到反射点的距离的平方值
        float light_intensity_attenuation = (light.position - point).dot(light.position - point);
 
       
        //计算漫反射，kd为漫反射系数，cwiseProduct()是矩阵点对点相乘，即哪行那列的数值乘以哪行哪列
        Eigen::Vector3f diffuse_reflection = kd.cwiseProduct(light.intensity / light_intensity_attenuation);
        diffuse_reflection *= reflection_intensity;
 
        //计算高光，ks为高光系数
        Eigen::Vector3f high_lights = ks.cwiseProduct(light.intensity / light_intensity_attenuation);
        high_lights *= std::pow(reflection_intensity,p);
 
        
        //漫反射、高光两个因素叠加得到最终效果
        result_color += (diffuse_reflection  + high_lights);
        
    }