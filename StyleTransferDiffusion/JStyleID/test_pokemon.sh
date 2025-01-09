STY_NUM=4

# for ((no = 0; no < $STY_NUM; no+=1)); do
#     python3 diffusers_implementation/run_styleid_diffusers.py \
#         --cnt_fn "test/Mewtwo.png" \
#         --sty_folder "../checker/B" \
#         --sty_no $no \
#         --result_fn "$no.png"
# done

for ((no = 0; no < $STY_NUM; no+=1)); do
    python3 diffusers_implementation/run_styleid_diffusers.py \
        --cnt_fn "test/Gyarados.png" \
        --sty_folder "../checker/A_gt" \
        --sty_no $no \
        --result_fn "$no.png"
done
