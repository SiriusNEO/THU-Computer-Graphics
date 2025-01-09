STY_NUM=5

for ((no = 0; no < $STY_NUM; no+=1)); do
    python3 diffusers_implementation/run_styleid_diffusers.py \
        --cnt_fn "test/cat.jpg" \
        --sty_folder "../checker/B" \
        --sty_no $no \
        --result_fn "$no.png"
done
