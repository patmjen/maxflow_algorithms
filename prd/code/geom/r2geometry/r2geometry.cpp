#include "r2geometry.h"
#include "geom/math.h"
#include "exttype/minmax.h"

namespace geom{
	namespace R2{
		using namespace exttype;
		//____________________________PointPair________________________________
		PointPair::PointPair(){
		};

		PointPair::PointPair(const vect2 & p1,const vect2 & p2):parent(p1,p2){
		};

		vect2 PointPair::v()const{
			return (*this)[1]-(*this)[0];
		};
		//_____________________________Segment_________________________________	
		Segment::Segment(){
		};

		Segment::Segment(const vect2 & p1,const vect2 & p2):parent(p1,p2){
		};

		double Segment::l()const{
			return v().norm();
		};

		int Segment::contains(const vect2 & p)const{
			//const Line & q = *this; //как это вообще компилируется?
			const Segment & q = *this;
			vect2 v = q.v().normalized();
			double x = (p-q[0])*v;
			return (math::abs(v^(p-q[0]))<eps0 && (x>-eps0) && (x<(q.v()*v)+eps0));
		};

		vect2& Segment::vertex(const int i){
			return (*this)[i];
		};

		const vect2& Segment::vertex(const int i)const{
			return (*this)[i];
		};

		double distance(const vect2 & p, const Segment & s){
			return p.distto(s[0],s[1]);
		};

		//_______________________________Line__________________________________
		Line::Line(){
		};

		Line::Line(const vect2 & p1,const vect2 & v):parent(p1,p1+v.normalized()){
		};

		Line::Line(const Segment & s):parent(s[0],s[0]+s.v().normalized()){
		};

		int Line::intersect(const Line & x)const{
			const Line & q = *this;
			const vect2 & qv0 = q[0];
			const vect2 & qv1 = q[1];
			const vect2 & xv0 = x[0];
			const vect2 & xv1 = x[1];
			return ( math::sgn((qv0-xv0)^x.v())!=math::sgn((qv1-xv0)^x.v()) &&  math::sgn((xv0-qv0)^q.v())!=math::sgn((xv1-qv0)^q.v()) );
		};

		double Line::intersectd(const Line & x)const{
			const Line & q = *this;
			vect2 v1 = q.v().normalized();
			vect2 v2 = x.v();
			const vect2 & xv0 = x[0];
			const vect2 & qv0 = q[0];
			return ((xv0-qv0)^v2)/(v1^v2);
		};

		vect2 Line::intersectPoint(const Line & x)const{
			const Line & q = *this;
			vect2 v1 = q.v().normalized();
			const vect2 & qv0 = q[0];
			return qv0 + v1*intersectd(x);
		};

		//_______________________________Rect__________________________________
		Rect::Rect():PointPair(vect2(0.0,0.0),vect2(0.0,0.0)){

		};

		Rect::Rect(const Rect & x):PointPair(x){
		};

		Rect::Rect(const vect2 & p1, const vect2 & p2):PointPair(p1,p2){
		};

		Rect::Rect(double left,double top,double width,double height):PointPair(vect2(left,top),vect2(left+width,top+height)){
		};

		Rect::Rect(const R1::Interval & Ix, const R1::Interval & Iy):PointPair(vect2(Ix[0],Iy[0]),vect2(Ix[1],Iy[1])){
		};

		void Rect::operator = (const Rect & r){
			PointPair::operator = (r);
		};

		Segment Rect::distance(const Rect & x)const{
			Segment mins;
			int f = 1;
			for(int i=0;i<4;i++){
				Segment s;
				s[0] = getCorner(i);
				for(int j=0;j<4;j++){
					int j1 = j+1;if(j1>=4)j1=0;
					s[1] = s[0].snaptoline(x.getCorner(j),x.getCorner(j1));
					if(f || s.l()<mins.l()){
						mins = s;
						f  = 0;
					};
				};
			};
			return mins;
		};

		vect2 Rect::getWidth()const{
			return vect2(v()[0],0.0);
		};

		vect2 Rect::getHeight()const{
			return vect2(0.0,v()[1]);
		};

		vect2 Rect::getCorner(int i)const{
			switch(i){
				case 0:return item(0);break;
				case 1:return item(0)+getWidth();break;
				case 2:return item(1);break;
				case 3:return item(0)+getHeight();break;
				//default:throwException("Just 0..3 corner is available.");
			};
			return vect2(0.0,0.0);
		};

		double Rect::left()const{
			return item(0)[0];
		};

		double Rect::right()const{
			return item(1)[0];
		};

		double Rect::top()const{
			return item(0)[1];
		};

		double Rect::bottom()const{
			return item(1)[1];
		};

		double Rect::height()const{
			return bottom()-top();
		};

		double Rect::width()const{
			return right()-left();
		};

		vect2 Rect::center()const{
			return (item(1)+item(0))/2.0;
		};

		vect2 Rect::size()const{
			return (item(1)-item(0));
		};

		R1::Interval Rect::Ix()const{
			return R1::Interval(item(0)[0],item(1)[0]);
		};

		R1::Interval Rect::Iy()const{
			return R1::Interval(item(0)[1],item(1)[1]);
		};

		int Rect::isEmpty()const{
			return (width()<eps0 || height()<eps0);
		};

		void Rect::addWidth(double dw){
			int i = dw>0?1:0;
			(*this)[i][0]+=dw;
		};

		void Rect::addHeight(double dh){
			int i = dh>0?1:0;
			(*this)[i][1]+=dh;
		};

		void Rect::expandHeight(double dh){
			double c = center()[1];
			double s = size()[1]/2;
			(*this)[0][1] = c-s-dh;
			(*this)[1][1] = c+s+dh;
		};

		void Rect::expandWidth(double dw){
			double c = center()[1];
			double s = size()[1]/2;
			(*this)[0][0] = c-s-dw;
			(*this)[1][0] = c+s+dw;		
		};

		void Rect::expand(double dl){
			vect2 c = center();
			vect2 s = size()/2;
			(*this)[0] = c-s-vect2(dl,dl);
			(*this)[1] = c+s+vect2(dl,dl);
		};

		Rect Rect::intersect(const Rect & x)const{
			return Rect(Ix().intersect(x.Ix()),Iy().intersect(x.Iy()));
		};
		Rect Rect::coverage(const Rect & x)const{
			return Rect(Ix().coverage(x.Ix()),Iy().coverage(x.Iy()));
		};

		Rect Rect::coverage(const vect2 & p)const{
			return Rect(Ix().coverage(p[0]),Iy().coverage(p[1]));
		};

		//________________Triangle___________________________
		Triangle::Triangle(const vect2 & p1, const vect2 & p2, const vect2 & p3):parent(p1,p2,p3){
		};

		int Triangle::HitTest(const vect2 & p)const{ //CCW+ hittest
			for(int i=0;i<3;i++){
				if( ((p-item(i))^(item((i+1)%3)-item(i))) < 0)return 0;
			};
			return 1;
		};

		int Triangle::orientation()const{//CCW+ orientation
			return math::sgn((item(2)-item(1))^(item(1)-item(0)));
		};

		Rect Triangle::getBoundRect()const{
			vect2 Ix = vect2(min(item(0)[0],item(1)[0],item(2)[0]),max(item(0)[0],item(1)[0],item(2)[0]));
			vect2 Iy = vect2(min(item(0)[1],item(1)[1],item(2)[1]),max(item(0)[1],item(1)[1],item(2)[1]));
			return Rect(Ix,Iy);//vect2(Ix[0],Iy[0]),vect2(Ix[1],Iy[1]));
		};

		/*
		//___________________Triangles____________________________
		Triangles::Triangles(){
		};
		int Triangles::HitTest(const vect2 & p)const{
			for(int i=0;i<count();i++)if((*this)[i].HitTest(p))return 1;
			return 0;
		};
		Rect Triangles::getBoundRect()const{
			if(count()==0)return Rect();
			Rect r = (*this)[0].getBoundRect();
			for(int i=1;i<count();i++){
				Rect r1 = (*this)[i].getBoundRect();
				r = r.coverage(r1);// Rect(vect2(min(r[0][0],r1[0][0]),min(r[0][1],r1[0][1])),vect2(max(r[1][0],r1[1][0]),max(r[1][1],r1[1][1])));
			};
			return r;
		};
		*/
		//__________________Box__________________________
		const mint2 Box::CCWiterator::ii[4]={mint2(0,0),mint2(1,0),mint2(1,1),mint2(0,1)};
	};
};
