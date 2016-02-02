#ifndef CAFFE_UTIL_BBOX_UTIL_H_
#define CAFFE_UTIL_BBOX_UTIL_H_

#include <stdint.h>
#include <cmath>  // for std::fabs and std::signbit
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "glog/logging.h"

#include "caffe/caffe.hpp"

namespace caffe {

typedef MultiBoxLossParameter_MatchType MatchType;
typedef map<int, vector<NormalizedBBox> > LabelBBox;

// Function used to sort NormalizedBBox, stored in STL container (e.g. vector),
// in ascend order based on the score value.
bool SortBBoxAscend(const NormalizedBBox& bbox1, const NormalizedBBox& bbox2);

// Function used to sort NormalizedBBox, stored in STL container (e.g. vector),
// in descend order based on the score value.
bool SortBBoxDescend(const NormalizedBBox& bbox1, const NormalizedBBox& bbox2);

// Function sued to sort pair<float, int>, stored in STL container (e.g. vector)
// in descend order based on the score (first) value.
bool SortScorePairAscend(const pair<float, int>& pair1,
                         const pair<float, int>& pair2);

// Function sued to sort pair<float, int>, stored in STL container (e.g. vector)
// in descend order based on the score (first) value.
bool SortScorePairDescend(const pair<float, int>& pair1,
                          const pair<float, int>& pair2);

// Compute the intersection between two bboxes.
void IntersectBBox(const NormalizedBBox& bbox1, const NormalizedBBox& bbox2,
                   NormalizedBBox* intersect_bbox);

// Compute bbox size.
float BBoxSize(const NormalizedBBox& bbox);

// Compute the jaccard (intersection over union IoU) overlap between two bboxes.
float JaccardOverlap(const NormalizedBBox& bbox1, const NormalizedBBox& bbox2);

// Encode a bbox according to a prior bbox.
void EncodeBBox(const NormalizedBBox& prior_bbox,
    const vector<float>& prior_variance,
    const NormalizedBBox& bbox, NormalizedBBox* encode_bbox);

// Decode a bbox according to a prior bbox.
void DecodeBBox(const NormalizedBBox& prior_bbox,
    const vector<float>& prior_variance,
    const NormalizedBBox& bbox, NormalizedBBox* decode_bbox);

// Decode a set of bboxes according to a set of prior bboxes.
void DecodeBBoxes(const vector<NormalizedBBox>& prior_bboxes,
    const vector<vector<float> >& prior_variances,
    const vector<NormalizedBBox>& bboxes,
    vector<NormalizedBBox>* decode_bboxes);

// Match prediction bboxes with ground truth bboxes.
void MatchBBox(const vector<NormalizedBBox>& gt,
    const vector<NormalizedBBox>& pred_bboxes, const int label,
    const MatchType match_type, const float overlap_threshold,
    vector<int>* match_indices, vector<float>* match_overlaps);

// Retrieve bounding box ground truth from gt_data.
//    gt_data: 1 x 1 x num_gt x 7 blob.
//    num_gt: the number of ground truth.
//    background_label_id: the label for background class which is used to do
//      santity check so that no ground truth contains it.
//    all_gt_bboxes: stores ground truth for each image. Label of each bbox is
//      stored in NormalizedBBox.
template <typename Dtype>
void GetGroundTruth(const Dtype* gt_data, const int num_gt,
      const int background_label_id,
      map<int, vector<NormalizedBBox> >* all_gt_bboxes);
// Store ground truth bboxes of same label in a group.
template <typename Dtype>
void GetGroundTruth(const Dtype* gt_data, const int num_gt,
      const int background_label_id, map<int, LabelBBox>* all_gt_bboxes);

// Get location predictions from loc_data.
//    loc_data: num x num_preds_per_location * num_loc_classes * 4 blob.
//    num: the number of images.
//    num_preds_per_location: number of predictions per location.
//    num_loc_classes: number of location classes. It is 1 if share_location is
//      true; and is equal to number of classes needed to predict otherwise.
//    share_location: if true, all classes share the same location prediction.
//    loc_preds: stores the location prediction, where each item contains
//      location prediction for an image.
template <typename Dtype>
void GetLocPredictions(const Dtype* loc_data, const int num,
      const int num_preds_per_class, const int num_loc_classes,
      const bool share_location, vector<LabelBBox>* loc_preds);

// Get confidence predictions from conf_data.
//    conf_data: num x num_preds_per_location * num_classes blob.
//    num: the number of images.
//    num_preds_per_location: number of predictions per location.
//    num_classes: number of classes.
//    conf_preds: stores the confidence prediction, where each item contains
//      confidence prediction for an image.
template <typename Dtype>
void GetConfidenceScores(const Dtype* conf_data, const int num,
      const int num_preds_per_class, const int num_classes,
      vector<map<int, vector<float> > >* conf_scores);

// Get prior bounding boxes from prior_data.
//    prior_data: 1 x 2 x num_priors * 4 x 1 blob.
//    num_priors: number of priors.
//    prior_bboxes: stores all the prior bboxes in the format of NormalizedBBox.
//    prior_variances: stores all the variances needed by prior bboxes.
template <typename Dtype>
void GetPriorBBoxes(const Dtype* prior_data, const int num_priors,
      vector<NormalizedBBox>* prior_bboxes,
      vector<vector<float> >* prior_variances);

// Get detection results from det_data.
//    det_data: 1 x 1 x num_det x 7 blob.
//    num_det: the number of detections.
//    background_label_id: the label for background class which is used to do
//      santity check so that no detection contains it.
//    all_detections: stores detection results for each class from each image.
template <typename Dtype>
void GetDetectionResults(const Dtype* det_data, const int num_det,
      const int background_label_id,
      map<int, LabelBBox>* all_detections);

// Do non maximum suppression given bboxes and scores.
//    bboxes: a set of bounding boxes.
//    scores: a set of corresponding confidences.
//    threshold: the threshold used in non maximu suppression.
//    top_k: if not -1, keep at most top_k picked indices.
//    reuse_overlaps: if true, use and update overlaps; otherwise, always
//      compute overlap.
//    overlaps: a temp place to optionally store the overlaps between pairs of
//      bboxes if reuse_overlaps is true.
//    indices: the kept indices of bboxes after nms.
void ApplyNMS(const vector<NormalizedBBox>& bboxes, const vector<float>& scores,
      const float threshold, const int top_k, const bool reuse_overlaps,
      map<int, map<int, float> >* overlaps, vector<int>* indices);

// Compute cumsum of a set of pairs.
void CumSum(const vector<pair<float, int> >& pairs, vector<int>* cumsum);

// Compute average precision given true positive and false positive vectors.
//    tp: contains pairs of scores and true positive.
//    num_pos: number of positives.
//    fp: contains pairs of scores and false positive.
//    ap_version: different way of computing Average Precision.
//      Check https://sanchom.wordpress.com/tag/average-precision/ for details.
//      11point: the 11-point interpolcated average precision.
//      Integral: the natural integral of the precision-recall curve.
//    prec: stores the computed precisions.
//    rec: stores the computed recalls.
//    ap: the computed Average Precision.
void ComputeAP(const vector<pair<float, int> >& tp, const int num_pos,
               const vector<pair<float, int> >& fp, const string ap_version,
               vector<float>* prec, vector<float>* rec, float* ap);

}  // namespace caffe

#endif  // CAFFE_UTIL_BBOX_UTIL_H_