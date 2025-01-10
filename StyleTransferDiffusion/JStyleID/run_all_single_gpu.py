import json, os, tqdm
import jittor as jt
import asyncio
from asyncio import Queue

from JDiffusion.pipelines import StableDiffusionPipeline

max_num = 28
dreambooth_lora_weights = "../JDiffusion/examples/dreambooth"
# dataset_root = "/scorpio/home/linchaofan/THU-Computer-Graphics/StyleTransferDiffusion/checker/A_gt"
dataset_root = "/scorpio/home/linchaofan/THU-Computer-Graphics/StyleTransferDiffusion/checker/B"
save_dir = "./results_B_1/"

with jt.no_grad():
    for tempid in tqdm.tqdm(range(0, max_num)):
        taskid = "{:0>2d}".format(tempid)
        pipe = StableDiffusionPipeline.from_pretrained("stabilityai/stable-diffusion-2-1").to("cuda:1")

        # pipe.load_lora_weights(f"{dreambooth_lora_weights}/style_B/style_{taskid}")

        # load json
        with open(f"{dataset_root}/{taskid}/prompt.json", "r") as file:
            prompts = json.load(file)

        for id, prompt in prompts.items():
            print("Run prompt: ", prompt)
            # image = pipe(prompt + f" in style_{taskid}", num_inference_steps=25, width=512, height=512).images[0]
            image = pipe(prompt, num_inference_steps=50, width=512, height=512).images[0]
            os.makedirs(f"{save_dir}/{taskid}", exist_ok=True)

            # intermediate results
            temp_img = f"{save_dir}/{taskid}/{prompt}.png"
            image.save(temp_img)

            # run StyleID
            command = f"CUDA_VISIBLE_DEVICES=1 python3 diffusers_implementation/run_styleid_diffusers.py --cnt_fn \"{temp_img}\" --sty_folder \"{dataset_root}\" --sty_no {tempid} --save_dir \"{save_dir}/{taskid}\" --result_fn \"{prompt}.png\""

            print(f"Executing command: {command}")
            os.system(command)
