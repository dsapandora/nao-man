#ifndef FieldLines_h_DEFINED
#define FieldLines_h_DEFINED

// System includes
#include <cmath>                       // use fabs, max, min
#include <vector>                      //
#include <list>                        //
#include <sstream>                     //
#include <iomanip> // setprecision for cout
#include <boost/shared_ptr.hpp>

class FieldLines;

#include "FieldConstants.h"

// Signifies that the angle between two lines could not be calculated
static const int BAD_ANGLE = -22354;
// Signifies that the distance between two points could not be calculated
static const int BAD_DISTANCE = -23523134;

// for percentColor(), ortho directions
enum TestDirection {TEST_UP, TEST_DOWN, TEST_LEFT, TEST_RIGHT};

struct linePoint;

#include "Common.h" //
#include "VisualFieldObject.h" //
#include "ConcreteFieldObject.h" //
#include "ConcreteCorner.h" //
#include "VisualCorner.h" //
#include "VisualLine.h"
#include "Utility.h" //
#include "NaoPose.h" // Used to estimate distances in the image
#include "Vision.h"
#include "Profiler.h"

static const int NO_EDGE = -3;

// Color Constants
static const int USED_VERT_POINT_COLOR = BLACK;
static const int UNUSED_VERT_POINT_COLOR = CYAN;
static const int FIT_VERT_POINT_COLOR = BROWN;

static const int USED_HOR_POINT_COLOR = RED;
static const int UNUSED_HOR_POINT_COLOR = PURPLE;
static const int FIT_HOR_POINT_COLOR = SEA_GREEN;

static const int TENTATIVE_INTERSECTION_POINT_COLOR = LAWN_GREEN;
static const int LEGIT_INTERSECTION_POINT_COLOR = ORANGE;
static const int INVALIDATED_INTERSECTION_POINT_COLOR = PURPLE;

static const int FIT_UNUSED_POINTS_BOX_COLOR = MAROON;
static const int JOIN_LINES_BOX_COLOR = PINK;

static const Rectangle SCREEN = {0, IMAGE_WIDTH - 1,
                                 0, IMAGE_HEIGHT - 1};
// More succinct.
typedef std::list<linePoint>::iterator linePointNode;

class FieldLines {
private:


    // Miscellaneous

    ////////////////////////////////////////////////////////////
    // Find Line Points
    ////////////////////////////////////////////////////////////
    // Constants
    // Used in vert and horizontal edge detect

    // Change in Y channel value over one pixel necessary to constitute an edge
    static const int VERTICAL_TRANSITION_VALUE = 10;
    static const int HORIZONTAL_TRANSITION_VALUE = 10;

    static const int NUM_TEST_PIXELS = 15;

    static const int NUM_GREEN_COLORS = 2;
    static const int FIELD_COLORS[NUM_GREEN_COLORS];
    static const int NUM_FIELD_COLORS = NUM_GREEN_COLORS;

    static const int NUM_WHITE_COLORS = 2;
    static const int LINE_COLORS[NUM_WHITE_COLORS];
    static const int NUM_LINE_COLORS = NUM_WHITE_COLORS;

    // Number of columns in which to search for line points
    static const int NUM_COLS_TO_TEST = 25;
    // Number of rows in which to search for line points
    static const int NUM_ROWS_TO_TEST = 25;
    // Number of pixels to skip between columns in image when searching for
    // linepoints vertically
    static const int COL_SKIP = IMAGE_WIDTH / NUM_COLS_TO_TEST;
    // Number of pixels to skip between rows in image when searching for
    // linepoints horizontally
    static const int ROW_SKIP = IMAGE_HEIGHT / NUM_ROWS_TO_TEST;

    // percentage of pixels needed to be green on either side of the line
    static const int GREEN_PERCENT_CLEARANCE = 40;

    // If we have seen an edge within the past x pixels, then we say it's close
    // AIBOSPECIFIC
    static const int ADJACENT_SAME_EDGE_SEPARATION = 3;
    // AIBO_SPECIFIC
    // if the edge detection goes bad on one end of the line, this is a check
    static const int NUM_NON_WHITE_SANITY_CHECK = 3;
    static const int NUM_UNDEFINED_SANITY_CHECK = 5;


    ////////////////////////////////////////////////////////////
    // Create Lines Constants
    ////////////////////////////////////////////////////////////

    // AIBO_SPECIFIC
    static const int MIN_PIXEL_WIDTH_FOR_GREEN_CHECK = 2;
    static const int MIN_SEPARATION_TO_NOT_CHECK = 10;
    // AIBOSPECIFIC
    // Two line points must have at least this Euclidean distance between
    // them in order for us to check their angle
    static const int MIN_PIXEL_DIST_TO_CHECK_ANGLE = 2;

    static const int MAX_ANGLE_LINE_SEGMENT = 4;

    static const int MAX_GREEN_PERCENT_ALLOWED_IN_LINE = 10;

    // max number of pixels offset to connect two points in createLines
    static const int GROUP_MAX_X_OFFSET = static_cast<int>(.30 * IMAGE_WIDTH);
    // NOTE: Currently Unused
    // max number of pixels offset to connect two y points
    //static const int GROUP_MAX_Y_OFFSET = static_cast<int>(.20 * IMAGE_WIDTH);

    ////////////////////////////////////////////////////////////
    // Join Lines Constants
    ////////////////////////////////////////////////////////////
    // NOTE: Currently Unused
    // static const int JOIN_MAX_X_OFFSET = static_cast<int>(.35 * IMAGE_WIDTH);
    // static const int JOIN_MAX_Y_OFFSET = static_cast<int>(.25 * IMAGE_WIDTH);
    static const int MAX_ANGLE_TO_JOIN_LINES = 9;
    static const int MIN_ANGLE_TO_JOIN_CC_LINES = 10;
    static const int MAX_ANGLE_TO_JOIN_CC_LINES = 45;
    static const int MAX_DIST_BETWEEN_TO_JOIN_LINES = 9;
    static const int MAX_DIST_BETWEEN_TO_JOIN_CC_LINES = 12;


    ////////////////////////////////////////////////////////////
    // Fit Unused Points Constants
    ////////////////////////////////////////////////////////////
    static const int MAX_VERT_FIT_UNUSED_WIDTH_DIFFERENCE = 2;
    // NOTE: Currently Unused
    //static const int MAX_HOR_FIT_UNUSED_WIDTH_DIFFERENCE = 2;

    ////////////////////////////////////////////////////////////
    // Extend Lines constants
    ////////////////////////////////////////////////////////////
    static const int MAX_EXTEND_LINES_WIDTH_DIFFERENCE = 20;

    ////////////////////////////////////////////////////////////
    // Intersect Lines constants
    ////////////////////////////////////////////////////////////
    static const int MAX_GREEN_PERCENT_ALLOWED_AT_CORNER = 70;

    // Two many duplicate intersection points indicate we are at the
    // center circle
    static const int MAX_NUM_DUPES = 0;

    // The bounding box extends some pixels on either side parallel to the line
    static const int INTERSECT_MAX_PARALLEL_EXTENSION =
        static_cast<int>(.15 * IMAGE_WIDTH);
    // the bounding box extends some pixels on either side perpendicular to the
    // line
    static const int INTERSECT_MAX_ORTHOGONAL_EXTENSION =
        static_cast<int>(.05 * IMAGE_WIDTH);
    // for dupeCorner() checks
    static const int DUPE_MIN_X_SEPARATION = 15;
    static const int DUPE_MIN_Y_SEPARATION = 15;

    static const int MAX_CORNER_DISTANCE = 600;
    static const int MIN_CORNER_DISTANCE = 10;

    static const int CORNER_TEST_RADIUS = 1;

    static const int MIN_ANGLE_BETWEEN_INTERSECTING_LINES = 15;
    static const int LINE_HEIGHT = 0; // this refers to height off the ground
    static const int MIN_CROSS_EXTEND = 20;
    // When estimating the angle between two lines on the field, anything less
    // than MIN_ANGLE_ON_FIELD or greater than MAX_ANGLE_ON_FIELD is suspect
    // and disallowed; ideally our estimates would always be 90.0 degrees
    static const int MIN_ANGLE_ON_FIELD = 55;
    static const int MAX_ANGLE_ON_FIELD = 115;
    static const int TWO_CORNER_LINES_MIN_LENGTH = 35;

    static const int DEBUG_GROUP_LINES_BOX_WIDTH = 4;

    ////////////////////////////////////////////////////////////
    // Identify corners constants
    ////////////////////////////////////////////////////////////

public:

    FieldLines(Vision *visPtr,
			   boost::shared_ptr<NaoPose> posePtr,
			   boost::shared_ptr<Profiler> profilerPtr);
    virtual ~FieldLines() {}

    // master loop
    void lineLoop();

    // While lineLoop is called before object recognition so that
    // ObjectFragments can make use of VisualLines and VisualCorners,
    // the methods called from here use FieldObjects and as such must be
    // performed after the ObjectFragments loop is completed.
    void afterObjectFragments();

    // This method populates the points vector with line points it finds in
    // the image.  A line point ideally occurs in the middle of a line on the
    // screen.  We detect lines via a simple edge detection scheme -
    // a transition from green to white involves a big positive jump in Y
    // channel, while a transition from white to green involves a big negative
    // jump in Y channel.
    //
    // The vertical in this method name refers to the fact that we start at the
    // bottom of the image and scan up for points.
    // @param vertLinePoints - the vector to fill with all points found in
    // the scan
    void findVerticalLinePoints(std::vector<linePoint> &vertLinePoints);

    // This method populates the points vector with line points it finds in
    // the image.  A line point ideally occurs in the middle of a line on the
    // screen.  We detect lines via a simple edge detection scheme -
    // a transition from green to white involves a big positive jump in Y
    // channel, while a transition from white to green involves a big negative
    // jump in Y channel.
    //
    // The horizontal in the method name denotes that we start at the left of
    // the image and scan to the right to find these points
    // @param horLinePoints - the vector to fill with all points found in
    // the scan
    void findHorizontalLinePoints(std::vector<linePoint> &horLinePoints);

    // Attempts to create lines out of a list of linePoints.  In order for
    // points to be fit onto a line, they must pass a battery of sanity checks
    void createLines(std::list<linePoint> &linePoints);

    void setLineCoordinates(boost::shared_ptr<VisualLine> aLine);

    // Attempts to fit the left over points that were not used within the
    // createLines function to the lines that were output from said function
    void fitUnusedPoints(std::vector< boost::shared_ptr<VisualLine> > &lines,
                         std::list<linePoint> &remainingPoints);

    // Attempts to join together line segments that are logically part of one
    // longer line but for some reason were not grouped within the groupPoints
    // method.  This can often happen when there is an obstruction that obscures
    // part of the line; due to x offset sanity checks, points that are too far
    // apart are not allowed to be within the same line in createLines.
    void joinLines();

    // Copies the data from line1 and 2 into a new single line.
    boost::shared_ptr<VisualLine> mergeLines(boost::shared_ptr<VisualLine> line1,
											 boost::shared_ptr<VisualLine> line2);

    // Given a vector of lines, attempts to extend the near vertical ones to the
    // top and bottom, and the more horizontal ones to the left and right
    void extendLines(std::vector< boost::shared_ptr<VisualLine> > &lines);

    // Helper method that returns true if the color is one we consider to be
    // a line color
    static const bool isLineColor(const int color);

    // Helper method that retursn true if the color is one we consider to be a
    // line color
    static const bool isGreenColor(int threshColor);

    // Given a line, attempts to extend it to both the left and right
    void extendLineHorizontally(boost::shared_ptr<VisualLine> line);

    // Given a line, attempts to extend it to both the top and bottom.
    void extendLineVertically(boost::shared_ptr<VisualLine> line);

    // Returns true if the new point trying to be added to the line is offscreen
    // or  there is too much green in between the old and new point.
    // Any further searching in this direction would be foolish.
    const bool shouldStopExtendingLine(const int oldX, const int oldY,
                                       const int newX, const int newY) const;

    // Given an (x, y) location and a direction (horizontal or vertical) in
    // which to look, attempts to find edges on either side of the (x,y)
    // location.  If there are no edges, or if another sanity check fails,
    // returns VisualLine::DUMMY_LINEPOINT.  Otherwise it returns the linepoint
    // with the correct (x,y) location and width and scan.
    linePoint findLinePointFromMiddleOfLine(int x, int y, ScanDirection dir);

    // Unlike our normal method for finding line points, this searches from the
    // middle of a line outward for an edge, in a given direction, up to a max
    // of maxPixelsToSearch.  If no edge is found, returns NO_EDGE.
    const int findEdgeFromMiddleOfLine(int x, int y, int maxPixelsToSearch,
                                       TestDirection dir) const;

	void removeDuplicateLines();

    // Pairwise tests each line on the screen against each other, calculates
    // where the intersection occurs, and then subjects the intersection
    // to a battery of sanity checks before determining that the intersection
    // is a legitimate corner on the field.
    // @param lines - the vector of visual lines that have been found after
    // createLines, join lines, and fit unused points.
    // @return a vector of VisualCorners created from the intersection points
    // that successfully pass all sanity checks.
    //
    std::list<VisualCorner> intersectLines();


	/**
	 * Sanity checks for field lines:
	 */
	const bool isAngleTooSmall(boost::shared_ptr<VisualLine> i,
						 boost::shared_ptr<VisualLine> j,
						 const int& numChecksPassed) const;

	const bool isIntersectionOnScreen(const point<int>& intersection,
								const int& numChecksPassed) const;

	const bool isAngleOnFieldOkay(boost::shared_ptr<VisualLine> i,
							boost::shared_ptr<VisualLine> j,
							const int& intersectX,
							const int& intersectY,
							const int& numChecksPassed) const;

	const bool tooMuchGreenAtCorner(const point<int>& intersection,
							  const int& numChecksPassed);

	const bool areLinesTooSmall(boost::shared_ptr<VisualLine> i,
						  boost::shared_ptr<VisualLine> j,
						  const int& numChecksPassed) const;

	const bool doLinesCross(boost::shared_ptr<VisualLine> i,
					  boost::shared_ptr<VisualLine> j,
					  const float& t_I, const float& t_J,
					  const int& numChecksPassed)const ;

	const bool isCornerTooFar(const float& distance,
						const int& numChecksPassed) const;

	const bool areLineEndsCloseEnough(boost::shared_ptr<VisualLine> i,
								boost::shared_ptr<VisualLine> j,
								const point<int>& intersection,
								const int& numChecksPassed) const;

	const bool tooMuchGreenEndpointToCorner(const point<int>& line1Closer,
									  const point<int>& line2Closer,
									  const point<int>& intersection,
									  const int& numChecksPassed) const;

	const bool isTActuallyCC(const VisualCorner& c,
							 boost::shared_ptr<VisualLine> i,
							 boost::shared_ptr<VisualLine> j,
							 const point<int>& intersection,
							 const point<int>& line1Closer,
							 const point<int>& line2Closer);

	// Checks if a corner is too dangerous when it is relatively near the edge
	// of the screen - scans the edge for a stripe of white
	const bool tooClose(int x, int y);

    // Iterates over the corners and removes those that are too risky to
    // use for localization data
    void removeRiskyCorners(//vector<VisualLine> &lines,
        std::list<VisualCorner> &corners);

    // Given a list of VisualCorners, attempts to assign ConcreteCorners
    // (ideally one, but sometimes multiple) that correspond with where the
    // corner could possibly be on the field.  For instance, if we have a T
    // corner and it is right next to the blue goal left post, then it is the
    // blue goal right T. Modifies the corners passed in by calling the
    // setPossibleCorners method; in certain cases the shape of a corner might
    // be switched too (if an L corner is determined to be a T instead, its
    // shape is changed accordingly).
    void identifyCorners(std::list<VisualCorner> &corners);

    const bool nearGoalTCornerLocation(const VisualCorner& corner,
                                       const VisualFieldObject * post) const;

    // Determines if the given L corner does not geometrically make sense for
    // its shape given the objects on the screen.
    const bool LCornerShouldBeTCorner(const VisualCorner &L) const;

    // In some Nao frames, robots obscure part of the goal and the bottom is not
    // visible.  We can only use pix estimates of goals whose bottoms are
    // visible
    const bool goalSuitableForPixEstimate(const VisualFieldObject * goal) const;

    // If it's a legitimate L, the post should be INSIDE of the two lines
    const bool LWorksWithPost(const VisualCorner& c,
                              const VisualFieldObject * post) const;

    void printFieldObjectsInformation();

    // Helper method that iterates over a list of ConcreteCorner pointers and
    // prints their string representations
    void printPossibilities(const std::list <const ConcreteCorner*> &list)const;

    int numPixelsToHitColor(const int x, const int y, const int colors[],
                            const int numColors,
                            const TestDirection testDir) const;
    int numPixelsToHitColor(const int x, const int y, const int color,
                            const TestDirection testDir) const;

    // Uses the actual objects' locations on the field to calculate straight
    // line distance
    float getRealDistance(const ConcreteCorner *c,
                          const VisualFieldObject *obj) const;

    // Estimates how long the line is on the field
    float getEstimatedLength(boost::shared_ptr<VisualLine> line) const;

    // Given two points on the screen, estimates the straight line distance
    // between them, on the field
    float getEstimatedDistance(const point<int> &point1,
                               const point<int> &point2) const;

    // Estimates the distance between the corner and the object based on
    // vectors
    float getEstimatedDistance(const VisualCorner *c,
                               const VisualFieldObject *obj) const;

    float getEstimatedAngle(const VisualCorner &corner) const;

    float getEstimatedAngle(boost::shared_ptr<VisualLine> line1,
                            boost::shared_ptr<VisualLine> line2,
                            const int intersectX,
                            const int intersectY) const;

	void classifyCornerWithObjects(
		const VisualCorner &corner,
		const std::vector <const VisualFieldObject*> &visibleObjects,
		list<const ConcreteCorner*>* classifications) const;

	std::list<const ConcreteCorner*>
	compareObjsCorners(const VisualCorner& corner,
					   const std::vector<const ConcreteCorner*>& possibleCorners,
					   const std::vector<const VisualFieldObject*>& visibleObjects)
		const;

	const bool arePointsCloseEnough(const float estimatedDistance,
									const ConcreteCorner* j,
									const VisualFieldObject* k) const;


    float getAllowedDistanceError(VisualFieldObject const *obj) const;







    // Return true if it appears the T is out of bounds, false otherwise
    /*
      bool isOutOfBoundsT(corner &t, int i);
    */

    const bool dupeCorner(const std::list<VisualCorner> &corners,
										const point<int>& intersection,
										const int testNumber) const;
	void removeDupeCorners(std::list<VisualCorner> &corners,
						   const point<int>& intersection);
	const bool dupeFakeCorner(const std::list<point <int> > &corners,
							  const int x, const int y, const int testNumber) const;
    const float percentColor(const int x, const int y, const TestDirection dir,
                             const int color, const int numPixels) const;
    const float percentColor(const int x, const int y, const TestDirection dir,
                             const int colors[], const int numColors,
                             const int numPixels) const;
    const float percentSurrounding(const int x, const int y,
                                   const int colors[], const int numColors,
                                   const int numPixels) const;
    const float percentSurrounding(const int x, const int y, const int color,
                                   const int numPixels) const;
    // Alternative form of percent surrounding that uses points.
    const float percentSurrounding(const point<int> &p,
                                   const int colors[], const int numColors,
                                   const int numPixels) const;

    const float percentColorBetween(const int x1, const int y1,
                                    const int x2, const int y2,
                                    const int colors[],
                                    const int numColors) const;
    const float percentColorBetween(const int x1, const int y1,
                                    const int x2, const int y2,
                                    const int color) const;


    void drawBox(BoundingBox box, int color) const;
    void drawSurroundingBox(boost::shared_ptr<VisualLine> aLine, int color) const;

    const bool isGreenWhiteEdge(int x, int y, ScanDirection direction) const;
    const bool isWhiteGreenEdge(int x, int y, int potentialMidPoint,
                                const ScanDirection direction) const;



    // Implementation can be found at revision 4518 if we ever want to get it
    // again.
    const int findCorrespondingBottom(const int x, const int y) const;

    static void updateLineCounters(const int threshColor, int &numWhite,
                                   int &numUndefined, int &numNonWhite);

#ifdef OFFLINE
    static void resetLineCounters(int &numWhite, int &numUndefined,
                                  int &numNonWhite);

    bool countersHitSanityChecks(const int numWhite, const int numUndefined,
                                 const int numNonWhite, const bool print) const;
#endif

    void drawFieldLine(boost::shared_ptr<VisualLine> _line, const int color) const;

    void drawLinePoint(const linePoint &p, const int color) const;
    void drawLinePoints(const std::list<linePointNode> &toDraw) const;
    void drawLinePoints(const std::list<linePoint> &toDraw) const;
    void drawCorners(const std::list<VisualCorner> &toDraw, int color);

    bool isLegitVerticalLinePoint(int x, int y);
#ifdef OFFLINE
    void setDebugVertEdgeDetect(bool _bool) { debugVertEdgeDetect = _bool; }
    void setDebugHorEdgeDetect(bool _bool) { debugHorEdgeDetect = _bool; }
    void setDebugSecondVertEdgeDetect(bool _bool) {
        debugSecondVertEdgeDetect = _bool;
    }
    void setDebugCreateLines(bool _bool) { debugCreateLines = _bool; }
    void setDebugFitUnusedPoints(bool _bool) { debugFitUnusedPoints = _bool; }
    void setDebugJoinLines(bool _bool) { debugJoinLines = _bool; }
    void setDebugExtendLines(bool _bool) { debugExtendLines = _bool; }
    void setDebugIntersectLines(bool _bool) { debugIntersectLines = _bool; }
    void setDebugIdentifyCorners(bool _bool) { debugIdentifyCorners = _bool; }
    void setDebugCcScan(bool _bool) { debugCcScan = _bool; }
    void setDebugRiskyCorners(bool _bool) { debugRiskyCorners = _bool; }

    void setDebugCornerAndObjectDistances(bool _bool) {
        debugCornerAndObjectDistances = _bool;
    }
    void setStandardView(bool _bool) {
        standardView = _bool;
    }

    const bool getDebugVertEdgeDetect() const { return debugVertEdgeDetect; }
    const bool getDebugHorEdgeDetect() const { return debugHorEdgeDetect; }
    const bool getDebugSecondVertEdgeDetect() const {
        return debugSecondVertEdgeDetect;
    }
    const bool getDebugCreateLines() const { return debugCreateLines; }
    const bool getDebugJoinLines() const { return debugJoinLines; }
    const bool getDebugFitUnusedPoints() const { return debugFitUnusedPoints; }
    const bool getDebugExtendLines() const { return debugExtendLines; }
    const bool getDebugIntersectLines() const { return debugIntersectLines; }
    const bool getDebugIdentifyCorners() const { return debugIdentifyCorners; }
    const bool getDebugCcScan() const { return debugCcScan; }
    const bool getDebugRiskyCorners() const { return debugRiskyCorners; }
    const bool getDebugCornerAndObjectDistances() const {
        return debugCornerAndObjectDistances;
    }
    const bool getStandardView() { return standardView; }
#endif

    const std::vector < boost::shared_ptr<VisualLine> >* getLines() const { return &linesList; }
    const std::list <VisualCorner>* getCorners() const {return &cornersList; }
    const int getNumCorners() { return cornersList.size(); }
    const std::list<linePoint>* getUnusedPoints() const {
        return &unusedPointsList;
    }

    // Returns true if the line segment drawn between first and second
    // intersects any field line on the screen; false otherwise
    const bool intersectsFieldLines(const point<int>& first,
                                    const point<int>& second) const;


#ifdef OFFLINE
    void printThresholdedImage();
#endif

    /* ----------------  Section for verbose helper methods ----------------
     * These methods are all really simple and their names are meant to be self
     * explanatory comments. This really helps keep down the length of
     * findVerticalLinePoints down to an acceptable minimum. They also take care
     * of printing debugging info and as such have to accept diverse parameters.
     */
private:
    static const int NUM_FIELD_OBJECTS_WITH_DIST_INFO = 4;
    VisualFieldObject const * allFieldObjects[NUM_FIELD_OBJECTS_WITH_DIST_INFO];

    // Determines which field objects are visible on the screen and returns
    // a vector of the pointers of the objects that are visible.
    std::vector<const VisualFieldObject*> getVisibleFieldObjects() const;

	vector<const VisualFieldObject*> getAllVisibleFieldObjects() const;

    // Returns whether there is a yellow post on screen that vision has not
    // identified the side of
    const bool unsureYellowPostOnScreen() const;
    // Returns whether there is a blue post on screen that vision has not
    // identified the side of
    const bool unsureBluePostOnScreen() const;

    // Returns whether there is a yellow post close to this corner
    const bool yellowPostCloseToCorner(VisualCorner& c);

    // Returns whether there is a blue post close to this corner
    const bool bluePostCloseToCorner(VisualCorner& c);

    const bool postOnScreen() const;

#ifdef OFFLINE
    static const bool isUphillEdge(const int, const int,
                                   const ScanDirection dir);
    static const bool isDownhillEdge(const int, const int,
                                     const ScanDirection dir);

    // Check to see if a particular variable holds a valid edge or the special
    // value of NO_EDGE.
    static inline const bool haveFound(const int edgeY) {
        return edgeY != NO_EDGE;
    }

    static const bool isAtTopOfImage(const int y, const int horizonY);
    static const bool isAtRightOfImage(const int x, const int endX);
    static const bool isWaitingForAnotherTopEdge(const int topEdgeY,
                                                 const int currentY);
    static const bool isWaitingForAnotherRightEdge(const int rightEdgeX,
                                                   const int currentX);

    // These cannot be static only because they access the debugging booleans.
    const bool isFirstUphillEdge(const int uphillEdgeLoc,
                                 const int x,
                                 const int y,
                                 const ScanDirection direction) const;
#endif
    const bool isSecondCloseUphillEdge(const int oldEdgeX,
                                       const int oldEdgeY,
                                       const int newEdgeX,
                                       const int newEdgeY,
                                       const ScanDirection direction) const;
    const bool isSecondFarUphillEdge(const int oldEdgeX,
                                     const int oldEdgeY,
                                     const int newX,
                                     const int newY,
                                     const ScanDirection direction) const;
    const bool isSecondUphillButInvalid(const int oldEdgeX,
                                        const int oldEdgeY,
                                        const int newEdgeX,
                                        const int newEdgeY,
                                        const ScanDirection dir) const;
#ifdef OFFLINE
    const bool isMoreSuitableTopEdge(const int topEdgeY,
                                     const int newY,
                                     const int imageColumn) const;
    const bool isMoreSuitableRightEdge(const int rightEdgeX,
                                       const int newX,
                                       const int y) const;


    void downhillEdgeWasTooFar(const int imageColumn, const int imageRow,
                               const ScanDirection dir) const;
    void secondDownhillButInvalid(const int imageColumn,
                                  const int imageRow,
                                  const ScanDirection dir) const;
    void foundDownhillNoUphill(const int imageColumn,
                               const int imageRow,
                               const ScanDirection dir) const;
    void couldNotFindCorrespondingBottom(const int imageColumn,
                                         const int imageRow) const;

    static const bool isEdgeClose(const int edgeLoc,
                                  const int newLoc);
#endif


    const bool isReasonableVerticalWidth(const int x, const int y,
                                         const float dist,
                                         const int width) const;
    const bool isReasonableHorizontalWidth(const int x, const int y,
                                           const float dist,
                                           const int width) const;





private:
    Vision *vision;
    boost::shared_ptr<NaoPose> pose;
    boost::shared_ptr<Profiler> profiler;

    std::vector <boost::shared_ptr<VisualLine> > linesList;
    std::list <VisualCorner> cornersList;
    std::list <linePoint> unusedPointsList;

private:

    // debug variables
#ifdef OFFLINE
    bool debugVertEdgeDetect;
    bool debugHorEdgeDetect;
    bool debugSecondVertEdgeDetect;
    bool debugCreateLines;
    bool debugJoinLines;
    bool debugIntersectLines;
    bool debugExtendLines;
    bool debugIdentifyCorners;
    bool debugCcScan;
    bool debugRiskyCorners;
    bool debugCornerAndObjectDistances;
    bool debugFitUnusedPoints;
    // Normal users of cortex do not need to see as much debugging information
    // as I have been drawing; now there will be fewer colors etc to keep
    // track of
    bool standardView;


    static const bool printLinePointInfo = false;
    static const char *linePointInfoFile;

#else
    static const bool debugVertEdgeDetect = false;
    static const bool debugHorEdgeDetect = false;
    static const bool debugSecondVertEdgeDetect = false;
    static const bool debugCreateLines = false;

    static const bool debugJoinLines = false;
    static const bool debugExtendLines = false;
    static const bool debugIntersectLines = false;
    static const bool debugIdentifyCorners = false;
    static const bool debugCcScan = false;
    static const bool debugRiskyCorners = false;
    static const bool debugCornerAndObjectDistances = false;
    static const bool debugFitUnusedPoints = false;

    static const bool standardView = false;

    static const bool printLinePointInfo = false;
    static const char *linePointInfoFile;
#endif
};


#endif // FieldLines_h_DEFINED
