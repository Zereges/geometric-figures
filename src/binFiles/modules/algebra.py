# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module contains several functions from linear algebra

def dotProduct(vector1, vector2):
	return sum(map(lambda z: z[0]*z[1], zip(vector1, vector2)))

def vectLen(vector):
	return sum(map(lambda x: x**2, vector))**0.5

def vectDiff(vector1, vector2):
	return map(lambda z:z[0]-z[1], zip(vector1, vector2))

def vectSum(vector1, vector2):
	return map(lambda z:z[0]+z[1], zip(vector1, vector2))

def vectMult(koef, vector):
	return map(lambda x: koef*x, vector)

def orthogonalizeVect(vector, orthonormalBasis):
	for basisVect in orthonormalBasis:
		lenInBasisVect=dotProduct(basisVect, vector)
		vector=vectSum(vector, vectMult(-lenInBasisVect, basisVect))
	return vector

def orthonormalizeBasis(basis):
	ortBasis=[]
	for vector in basis:
		vector=orthogonalizeVect(vector, ortBasis)
		vectorLen=vectLen(vector)
		if vectorLen>0.00001:
			vector=vectMult(1/vectorLen, vector)
			ortBasis.append(vector)
	return ortBasis

def orthonormalBasisFromPoints(points):
	points=list(points)
	point=points.pop()
	return orthonormalizeBasis(vectDiff(p,point) for p in points)


class Hyperplane:

	# Creates Hyperplane from its normal and position within it
	# (normalPos * normal = point in the Hyperplane)
	def __init__(self, normal, normalPos=0):
		l=vectLen(normal)
		self.normal=map(lambda x: x/l, normal)
		self.normalPos=normalPos*l

	# Distance in the direction of the normal
	def orientedDistance(self, coords):
		return dotProduct(self.normal, coords)-self.normalPos

