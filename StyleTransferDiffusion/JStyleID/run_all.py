import json, os, tqdm
import jittor as jt
import asyncio
from asyncio import Queue

from JDiffusion.pipelines import StableDiffusionPipeline

gpu_num = 4
max_num = 28
dreambooth_lora_weights = "../JDiffusion/examples/dreambooth"
# dataset_root = "/scorpio/home/linchaofan/THU-Computer-Graphics/StyleTransferDiffusion/checker/A_gt"
dataset_root = "/scorpio/home/linchaofan/THU-Computer-Graphics/StyleTransferDiffusion/checker/B"
save_dir = "./results_B_wo_db/"

async def text2image_worker(consumer_num, queue):
    with jt.no_grad():
        for tempid in tqdm.tqdm(range(0, max_num)):
            taskid = "{:0>2d}".format(tempid)
            pipe = StableDiffusionPipeline.from_pretrained("stabilityai/stable-diffusion-2-1").to("cuda")

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

                await asyncio.sleep(0.1)

                # run StyleID
                command = f"python3 diffusers_implementation/run_styleid_diffusers.py --cnt_fn \"{temp_img}\" --sty_folder \"{dataset_root}\" --sty_no {tempid} --save_dir \"{save_dir}/{taskid}\" --result_fn \"{prompt}.png\""
                await queue.put(command)

    # End signal
    for _ in range(consumer_num):
        await queue.put(None)

async def run_command(command):
    process = await asyncio.create_subprocess_shell(
        command,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE
    )

    stdout, stderr = await process.communicate()

    if stdout:
        print(f'Stdout: {stdout.decode().strip()}')
    if stderr:
        print(f'Stderr: {stderr.decode().strip()}')

    return process.returncode

async def styleid_worker(gpu_id, queue):
    while True:
        command = await queue.get()
        if command is None:
            break
        print(f"GPU {gpu_id} Consumed command: {command}")
        # os.system(command)
        await run_command(f"CUDA_VISIBLE_DEVICES={gpu_id} " + command)
        queue.task_done()

async def main():
    queue = Queue()

    consumer_num = gpu_num - 1
    for i in range(consumer_num):
        consumer = asyncio.create_task(styleid_worker(i+1, queue))
    producer = asyncio.create_task(text2image_worker(consumer_num, queue))
    await asyncio.gather(producer, consumer)

asyncio.run(main())
