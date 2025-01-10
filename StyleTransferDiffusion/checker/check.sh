# upload_path="$PWD/../JStyleID/results_A/"
# upload_path="$PWD/../JStyleID/results_B/"
# upload_path="$PWD/../JStyleID/results_A_wo_db/"
upload_path="$PWD/../JStyleID/results_B_wo_db/"
# upload_path="$PWD/../JDiffusion/examples/dreambooth/output_A/"
## upload_path="$PWD/../JDiffusion/examples/dreambooth/output_B/"
result_path="$PWD/result/"

# gt_path="$PWD/A_gt/"
gt_path="$PWD/B/"

echo "Start eval..."
echo "upload_path=$upload_path"
echo "gt_path=$gt_path"
echo "result_path=$result_path"

cd jdiff_checker
python3 score_api.py --upload_path=$upload_path --gt_path=$gt_path  --result_path=$result_path
