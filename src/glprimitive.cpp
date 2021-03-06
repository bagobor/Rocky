#include "glprimitive.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLPrimitive::GLPrimitive(float3 &tess, float3 &translate, float3 &scale) : vertexId_(0), indexId_(0) {
    translate_ = translate;
    scale_ = scale;
}

GLPrimitive::~GLPrimitive() {
    if(vertexId_) glDeleteBuffers(1, &vertexId_);
    if(indexId_) glDeleteBuffers(1, &indexId_);
}

void GLPrimitive::draw(GLShaderProgram *program, int instances) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    GLint ids[3] = {
	program->getAttributeLocation("in_Position"),
	program->getAttributeLocation("in_Normal"),
	program->getAttributeLocation("in_TexCoord")
    };
    if(ids[0] >= 0)
	glVertexAttribPointer(ids[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)vOffset_);
    if(ids[1] >= 0)
	glVertexAttribPointer(ids[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)nOffset_);
    if(ids[2] >= 0)
	glVertexAttribPointer(ids[2], 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)tOffset_);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if(ids[0] >= 0) glEnableVertexAttribArray(ids[0]);
    if(ids[1] >= 0) glEnableVertexAttribArray(ids[1]);
    if(ids[2] >= 0) glEnableVertexAttribArray(ids[2]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    if(type_ == GL_PATCHES) glPatchParameteri(GL_PATCH_VERTICES, typeCount_);
    glDrawElementsInstanced(type_, idxCount_, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0), instances);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if(ids[0] >= 0) glDisableVertexAttribArray(ids[0]);
    if(ids[1] >= 0) glDisableVertexAttribArray(ids[1]);
    if(ids[2] >= 0) glDisableVertexAttribArray(ids[2]);
}

void GLPrimitive::draw(GLShaderProgram *program) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    
    GLint ids[3] = {
	program->getAttributeLocation("in_Position"),
	program->getAttributeLocation("in_Normal"),
	program->getAttributeLocation("in_TexCoord")
    };
    if(ids[0] >= 0)
	glVertexAttribPointer(ids[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)vOffset_);
    if(ids[1] >= 0)
	glVertexAttribPointer(ids[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)nOffset_);
    if(ids[2] >= 0)
	glVertexAttribPointer(ids[2], 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)tOffset_);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if(ids[0] >= 0) glEnableVertexAttribArray(ids[0]);
    if(ids[1] >= 0) glEnableVertexAttribArray(ids[1]);
    if(ids[2] >= 0) glEnableVertexAttribArray(ids[2]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    if(type_ == GL_PATCHES) glPatchParameteri(GL_PATCH_VERTICES, typeCount_);
    
    glDrawElements(type_, idxCount_, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    if(ids[0] >= 0) glDisableVertexAttribArray(ids[0]);
    if(ids[1] >= 0) glDisableVertexAttribArray(ids[1]);
    if(ids[2] >= 0) glDisableVertexAttribArray(ids[2]);
}

void GLPrimitive::draw() {
     glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
     glEnableClientState(GL_VERTEX_ARRAY);
     glVertexPointer(3, GL_FLOAT, sizeof(GLVertex), BUFFER_OFFSET(vOffset_));  
     glEnableClientState(GL_NORMAL_ARRAY);
     glNormalPointer(GL_FLOAT, sizeof(GLVertex), BUFFER_OFFSET(nOffset_)); 
     glClientActiveTexture(GL_TEXTURE0);
     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
     glTexCoordPointer(3, GL_FLOAT, sizeof(GLVertex), BUFFER_OFFSET(tOffset_));
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
     glDrawElements(type_, idxCount_, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
     glDisableClientState(GL_VERTEX_ARRAY);
     glBindBuffer(GL_ARRAY_BUFFER, 0);
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

GLQuad::GLQuad(float3 tess, float3 translate, float3 scale) : GLPrimitive(tess, translate, scale) {
     this->tesselate(tess, translate, scale);
}


void GLQuad::tesselate(float3 tess, float3 translate, float3 scale) {

    if(vertexId_) glDeleteBuffers(1, &vertexId_);
    if(indexId_) glDeleteBuffers(1, &indexId_);


    type_ = GL_QUADS;
    idxCount_ = 4 * tess.x * tess.y;
    float3 delta = scale / tess;
    float3 tdelta = 1.0 / tess;
    delta.z = 0;

    GLVertex *pVertex = new GLVertex[(int)((tess.x + 1) * (tess.y + 1))];
    for(int y=0, i=0; y<=tess.y; y++) {
	for(int x=0; x<=tess.x; x++, i++) {
	    pVertex[i].p = float3(-0.5, -0.5, 0.0) * scale  + translate + delta * float3(x, y, 0);
	    pVertex[i].n = float3(0.0, 0.0, 1.0);
	    pVertex[i].t = float3(x, tess.y - y, 0) * tdelta;
	}
    }
    glGenBuffers(1, &vertexId_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex)*((tess.x + 1) * (tess.y + 1)), &pVertex[0].p.x, GL_STATIC_DRAW);

    unsigned short *pIndices = new unsigned short[idxCount_];
    for(int y=0, i=0; y<tess.y; y++) {
	for(int x=0; x<tess.x; x++, i+=4) {
	   pIndices[i] = y*(tess.x+1)+x;
	   pIndices[i+1] = y*(tess.x+1)+x+1;
	   pIndices[i+2] = (y+1)*(tess.x+1)+x+1;
	   pIndices[i+3] = (y+1)*(tess.x+1)+x;

	}
    }

    glGenBuffers(1, &indexId_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*idxCount_, &pIndices[0], GL_STATIC_DRAW);

    delete[] pVertex, delete[] pIndices;

    vOffset_ = 0;
    nOffset_ = 12;
    tOffset_ = 24;
}

GLPlane::GLPlane(float3 tess, float3 translate, float3 scale) : GLPrimitive(tess, translate, scale) {
     this->tesselate(tess, translate, scale);
}

void GLPlane::tesselate(float3 tess, float3 translate, float3 scale) {

    if(vertexId_) glDeleteBuffers(1, &vertexId_);
    if(indexId_) glDeleteBuffers(1, &indexId_);


    type_ = GL_TRIANGLES;
    idxCount_ = 6 * tess.x * tess.z;
    float3 delta = scale / tess;
    float3 tdelta = 1.0 / tess;
    delta.y = 0;
    GLVertex *pVertex = new GLVertex[(int)((tess.x + 1) * (tess.z + 1))];
    for(int z=0, i=0; z<=tess.z; z++) {
	for(int x=0; x<=tess.x; x++, i++) {
	    pVertex[i].p = float3(-0.5, 0.0, -0.5) * scale + translate + delta * float3(x, 0, z);
	    pVertex[i].n = float3(0.0, 1.0, 0.0);
	    pVertex[i].t = float3(x, z, 0) * tdelta;
	}
    }
    glGenBuffers(1, &vertexId_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex)*((tess.x + 1) * (tess.z + 1)), &pVertex[0].p.x, GL_STATIC_DRAW);

    unsigned short *pIndices = new unsigned short[idxCount_];
    for(int y=0, i=0; y<tess.z; y++) {
	for(int x=0; x<tess.x; x++, i+=6) {
	   pIndices[i] = y*(tess.x+1)+x;
	   pIndices[i+1] = y*(tess.x+1)+x+1;
	   pIndices[i+2] = (y+1)*(tess.x+1)+x+1;

	   pIndices[i+3] = y*(tess.x+1)+x;
	   pIndices[i+4] = (y+1)*(tess.x+1)+x+1;
	   pIndices[i+5] = (y+1)*(tess.x+1)+x;

	}
    }

    glGenBuffers(1, &indexId_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*idxCount_, &pIndices[0], GL_STATIC_DRAW);

    delete[] pVertex, delete[] pIndices;

    vOffset_ = 0;
    nOffset_ = 12;
    tOffset_ = 24;
}

GLIcosohedron::GLIcosohedron(float3 tess, float3 translate, float3 scale) : GLPrimitive(tess, translate, scale) {
    this->tesselate(tess, translate, scale);
}

void GLIcosohedron::tesselate(float3 tess, float3 translate, float3 scale) {
    
    if(vertexId_) glDeleteBuffers(1, &vertexId_);
    if(indexId_) glDeleteBuffers(1, &indexId_);
    
    type_ = GL_PATCHES;	
    typeCount_ = 3;

    const unsigned short pIndices[] = {
	2,1,0,3,2,0,4,3,0,5,4,0,1,5,0,11,6,7,11,7,8,11,8,9,11,9,10,11,10,6,1,2,6,
	2,3,7,3,4,8,4,5,9,5,1,10,2,7,6,3,8,7,4,9,8,5,10,9,1,6,10
    };

    GLVertex pVertex[12];
    pVertex[0].p = pVertex[0].n = float3(0.000f,  0.000f, 1.000f);
    pVertex[1].p = pVertex[1].n = float3(0.894f,  0.000f,  0.447f);
    pVertex[2].p = pVertex[2].n = float3(0.276f,  0.851f,  0.447f);
    pVertex[3].p = pVertex[3].n = float3(-0.724f,  0.526f,  0.447f);
    pVertex[4].p = pVertex[4].n = float3(-0.724f, -0.526f,  0.447f);
    pVertex[5].p = pVertex[5].n = float3( 0.276f, -0.851f,  0.447f);
    pVertex[6].p = pVertex[6].n = float3( 0.724f,  0.526f, -0.447f);
    pVertex[7].p = pVertex[7].n = float3(-0.276f,  0.851f, -0.447f);
    pVertex[8].p = pVertex[8].n = float3(-0.894f,  0.000f, -0.447f);
    pVertex[9].p = pVertex[9].n = float3(-0.276f, -0.851f, -0.447f);
    pVertex[10].p = pVertex[10].n = float3(0.724f, -0.526f, -0.447f);
    pVertex[11].p = pVertex[11].n = float3(0.000f,  0.000f, -1.000f);

    for(int i=0; i<12; i++) {
	pVertex[i].p *= scale;
	pVertex[i].p += translate;
    }

    idxCount_ = sizeof(pIndices) / sizeof(pIndices[0]);

    glGenBuffers(1, &vertexId_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex)*12, pVertex, GL_STATIC_DRAW);
    glGenBuffers(1, &indexId_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*idxCount_, pIndices, GL_STATIC_DRAW);
    
    vOffset_ = 0;
    nOffset_ = 12;
    tOffset_ = 24;
}


GLRect::GLRect(float3 tess, float3 translate, float3 scale) : GLPrimitive(tess, translate, scale) {
    this->tesselate(tess, translate, scale);
}

void GLRect::tesselate(float3 tess, float3 translate, float3 scale) {
    if(vertexId_) glDeleteBuffers(1, &vertexId_);
    if(indexId_) glDeleteBuffers(1, &indexId_);
    
    type_ = GL_PATCHES;
    typeCount_ = 4;
    idxCount_ = 4 * tess.x * tess.z;
    float3 delta = scale / tess;
    float3 tdelta = 1.0 / tess;
    delta.y = 0;
    GLVertex *pVertex = new GLVertex[(int)((tess.x + 1) * (tess.z + 1))];
    for(int z=0, i=0; z<=tess.z; z++) {
	for(int x=0; x<=tess.x; x++, i++) {
	    pVertex[i].p = float3(-0.5, 0.0, -0.5) * scale + translate + delta * float3(x, 0, z);
	    pVertex[i].n = float3(0.0, 1.0, 0.0);
	    pVertex[i].t = float3(x*tdelta.x, z*tdelta.z, 0);
	}
    }
    glGenBuffers(1, &vertexId_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex)*((tess.x + 1) * (tess.z + 1)), &pVertex[0].p.x, GL_STATIC_DRAW);

    unsigned short *pIndices = new unsigned short[idxCount_];
    for(int y=0, i=0; y<tess.z; y++) {
	for(int x=0; x<tess.x; x++, i+=4) {
	   pIndices[i] = y*(tess.x+1)+x;
	   pIndices[i+1] = y*(tess.x+1)+x+1;
	   pIndices[i+2] = (y+1)*(tess.x+1)+x+1;
	   pIndices[i+3] = (y+1)*(tess.x+1)+x;
	}
    }

    glGenBuffers(1, &indexId_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*idxCount_, &pIndices[0], GL_STATIC_DRAW);

    delete[] pVertex, delete[] pIndices;

    vOffset_ = 0;
    nOffset_ = 12;
    tOffset_ = 24;
}
