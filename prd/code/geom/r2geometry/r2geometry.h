#ifndef geom_r2_geometry_h
#define geom_r2_geometry_h

#include "vect2.h"
#include "exttype/arrays.h"
#include "../r1geometry/r1geometry.h"

namespace geom{
	namespace R2{
		//____________________________PointPair________________________________
		class PointPair : public pair<vect2>{
		public:
			typedef pair<vect2> parent;
		public:
			PointPair();
			PointPair(const vect2 & p1,const vect2 & p2);
			vect2 v()const;
		};
		//_____________________________Segment_________________________________
		class Segment: public PointPair{
		public:
			typedef PointPair parent;
		public:
			Segment();
			Segment(const vect2 & p1,const vect2 & p2);
			double l()const;
			int contains(const vect2 & p)const;
			vect2& vertex(const int i);
			const vect2& vertex(const int i)const;
		};

		double distance(const vect2 & p, const Segment & s);
		//______________________________Line___________________________________
		class Line : public PointPair{
		public:
			typedef PointPair parent;
		public:
			Line();
			Line(const vect2 & p1,const vect2 & v);
			Line(const Segment & s);
			int intersect(const Line & x)const;
			double intersectd(const Line & x)const;
			vect2 intersectPoint(const Line & x)const;
		};

		//______________________________Rect___________________________________
		class Rect: public PointPair{
		public:
			Rect();
			Rect(const Rect & x);
			Rect(const vect2 & p1, const vect2 & p2);
			Rect(double left,double top,double width,double height);
			Rect(const R1::Interval & Ix, const R1::Interval & Iy);
			void operator = (const Rect & r);
			Segment distance(const Rect & x)const;
			vect2 getWidth()const;
			vect2 getHeight()const;
			vect2 getCorner(int i)const;

			double left()const;
			double right()const;
			double top()const;
			double bottom()const;
			double height()const;
			double width()const;
			vect2 center()const;
			vect2 size()const;
			R1::Interval Ix()const;
			R1::Interval Iy()const;

			void addWidth(double dw);// dw>0?right+=dw:left+=dw
			void addHeight(double dh);// dh>0?bottom+=dh:top+=dh

			void expandHeight(double dh);
			void expandWidth(double dw);
			void expand(double dl);

			Rect intersect(const Rect & x)const;
			Rect coverage(const Rect & x)const;
			Rect coverage(const vect2 & p)const;

			int isEmpty()const;
		};

		//___________________Triangle_____________________________
		class Triangle : public fixedlist<vect2,3>{
		private:
			typedef fixedlist<vect2,3> parent;
		public:
			Triangle(){};
			Triangle(const vect2 & p1, const vect2 & p2, const vect2 & p3);
			int HitTest(const vect2 & p)const;
			int orientation()const;
			Rect getBoundRect()const;
		};

		/*
		//___________________Triangles_____________________________
		class Triangles : public _list::ownobjectlist<Triangle>{
		public:
			Triangles();
			int HitTest(const vect2 & p)const;
			Rect getBoundRect()const;
		};
		*/
		//___________________Box______________________
		class Box{
		public:
			class CCWiterator{
			private:
				static const mint2 ii[4];
				typedef mint2 parent;
			public:
				int i;
			public:
				CCWiterator():i(0){};
				void operator ++(){++i;};
				int allowed(){return i<4;};
				operator const mint2&(){return ii[i];};
			};
		};
	};	
};
#endif
